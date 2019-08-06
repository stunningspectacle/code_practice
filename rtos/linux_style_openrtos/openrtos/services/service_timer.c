/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2017 Intel Corporation. All Rights Reserved.

   The source code contained or described herein and all documents related
   to the source code ("Material") are owned by Intel Corporation or its
   suppliers or licensors. Title to the Material remains with Intel Corporation
   or its suppliers and licensors. The Material contains trade secrets and
   proprietary and confidential information of Intel or its suppliers and
   licensors. The Material is protected by worldwide copyright and trade secret
   laws and treaty provisions. No part of the Material may be used, copied,
   reproduced, modified, published, uploaded, posted, transmitted, distributed,
   or disclosed in any way without Intel's prior express written permission.

   No license under any patent, copyright, trade secret or other intellectual
   property right is granted to or conferred upon you by disclosure or delivery
   of the Materials, either expressly, by implication, inducement, estoppel or
   otherwise. Any license under such intellectual property rights must be
   express and approved by Intel in writing.
--*/

#include <limits.h>
#include <os_api.h>
#include <syslog.h>
#include <event_flag_defs.h>
#include <timedefs.h>
#include <service_timer.h>
#include <ish_bsp_timer.h>
#include <hpet.h>

//#define ENABLE_SERVICE_TIMER_DEBUG

#ifdef ENABLE_SERVICE_TIMER_DEBUG
#define SERVICE_TIMER_DEBUG(fmt, ...) syslog(LOG_INFO|LOG_COMP_SERVICES, "[Service Timer] " fmt, ##__VA_ARGS__)
#else
#define SERVICE_TIMER_DEBUG(fmt, ...) do {} while(0)
#endif

// resource limit
#define TASK_STACK_SIZE     (1200)
#define TASK_PRIORITY       (1)
#define TIMER_NUM_LIMIT     (20)

#define MASK_START          (1<<0)
#define MASK_RESET          (1<<1)
#define MASK_STOP           (1<<2)
#define MASK_KILL           (1<<3)
#define MASK_ISR            (1<<4)

#define CHECK_TIMER_ID(id) \
    do {\
        if ((id < 0) || (id > TIMER_NUM_LIMIT)) { \
            return -1; \
        } \
        if (0 == is_timer_inited(id)) {\
            return -2; \
        }\
    } while(0)

struct _ctrl_ {
    int module_inited;
    int bsp_timer_id;
    event_flag_handle_t event;
    LOCK_TYPE lock;
    int in_task;
};

struct _timer_ {
    int inited;
    uint64_t us;
    uint64_t us_ticks;
    uint64_t expires;
    timer_callback_f cb;
    void *param;
    int one_shot;
    int active;
};

static struct _ctrl_ controller;

static struct _timer_ timerset[TIMER_NUM_LIMIT];

static int find_free_timer(void)
{
    int i;

    Take_LOCK(controller.lock); // Good design!

    for (i = 0; i < TIMER_NUM_LIMIT; i++) {
        if (0 == timerset[i].inited) {
            break;
        }
    }

    Give_LOCK(controller.lock);

    return (TIMER_NUM_LIMIT == i) ? -1 : i;
}

static int is_timer_inited(int timer_id)
{
    Take_LOCK(controller.lock);

    int ret = timerset[timer_id].inited;

    Give_LOCK(controller.lock);

    return (1 == ret) ? 1 : 0;
}

int service_init_timer(IN uint64_t microseconds, IN timer_callback_f callback, IN void *param, IN bool one_shot)
{
    // SIMPLE LIMIT, see bsp_init_timer() 
    if ((microseconds > ULONG_MAX) || (NULL == callback)) {
        return -1;
    }

    int index = find_free_timer();
    if (-1 == index) {
        return -1;
    }

    Take_LOCK(controller.lock);
    
    timerset[index].inited = 1;
    timerset[index].us = microseconds;
    timerset[index].us_ticks = US_TO_HPET_CYCLE(microseconds);
    timerset[index].expires = 0;
    timerset[index].cb = callback;
    timerset[index].param = (void*)param;
    timerset[index].one_shot = one_shot;
    timerset[index].active = 0;

    Give_LOCK(controller.lock);

    return index;
}

/*
 * If start before expires, it will cancel and start with a new value
 */
int service_reset_timer(IN int timer_id, IN uint64_t microseconds)
{
    CHECK_TIMER_ID(timer_id);

    int flag_notify = 0;

    Take_LOCK(controller.lock);

    timerset[timer_id].us = microseconds;
    timerset[timer_id].us_ticks = US_TO_HPET_CYCLE(microseconds);
    timerset[timer_id].expires = CURRENT_HPET_CYCLE + timerset[timer_id].us_ticks;

    if (timerset[timer_id].active && (0 == controller.in_task)) {
        flag_notify = 1;
    }

    Give_LOCK(controller.lock);

    if (flag_notify) {
        event_flag_signal_handle(controller.event, MASK_RESET, EVENT_FLAG_OR);
    }

    return 0;
}

/*
 * If start before expires, it will cancel and start with a new value
 */
int service_start_timer(IN int timer_id)
{
    CHECK_TIMER_ID(timer_id);

    int flag_notify = 0;

    Take_LOCK(controller.lock);

    timerset[timer_id].active = 1;
    timerset[timer_id].expires = CURRENT_HPET_CYCLE + timerset[timer_id].us_ticks;

    if (0 == controller.in_task) {
        flag_notify = 1;
    }

    Give_LOCK(controller.lock);

    if (flag_notify) {
        event_flag_signal_handle(controller.event, MASK_START, EVENT_FLAG_OR);
    }

    return 0;
}

int service_stop_timer(IN int timer_id)
{
    CHECK_TIMER_ID(timer_id);

    int flag_notify = 0;

    Take_LOCK(controller.lock);

    if (timerset[timer_id].active) {
        timerset[timer_id].active = 0;
        if (0 == controller.in_task) {
            flag_notify = 1;
        }
    }

    Give_LOCK(controller.lock);

    if (flag_notify) {
        event_flag_signal_handle(controller.event, MASK_STOP, EVENT_FLAG_OR);
    }

    return 0;
}

int service_kill_timer(IN int timer_id)
{
    CHECK_TIMER_ID(timer_id);

    int flag_notify = 0;

    Take_LOCK(controller.lock);

    if (timerset[timer_id].active) {
        memset(&timerset[timer_id], 0, sizeof(struct _timer_));
        if (0 == controller.in_task) {
            flag_notify = 1;
        }
    }

    Give_LOCK(controller.lock);

    if (flag_notify) {
        event_flag_signal_handle(controller.event, MASK_KILL, EVENT_FLAG_OR);
    }

    return 0;
}

extern int bsp_timer_set_expires_and_start(const int id, const uint64_t expires);

static void daemon(void *arg)
{
    (void)arg;

    uint64_t last_expires_set = 0;

    while (1) {

        unsigned int event_mask = 0xFFFFFFFF;
        unsigned int actual_flags = 0;

        int result = event_flag_reltimedwait_handle(controller.event, event_mask, EVENT_FLAG_OR_CLEAR, &actual_flags, -1);
        if (0 != result) {
            syslog(LOG_ERR | LOG_COMP_SERVICES, "[Service timer] signal wait failed\n");
        }
        else {

            Take_LOCK(controller.lock);
            
            controller.in_task = 1;

            uint64_t curtick = CURRENT_HPET_CYCLE;
            uint64_t minimum_expires = ULLONG_MAX;

            for (int i = 0; i < TIMER_NUM_LIMIT; i++) {

                if ((0 == timerset[i].inited) || (0 == timerset[i].active)) {
                    continue;
                }

                // if need update
                if (timerset[i].expires <= curtick) {
                    // default update
                    if (1 == timerset[i].one_shot) {
                        timerset[i].active = 0;
                    }
                    else {
                        timerset[i].expires = curtick + timerset[i].us_ticks;
                    }
                    // user update
                    Give_LOCK(controller.lock); // callback MUST not be placed in critical context

                    timerset[i].cb(timerset[i].param); // if user restart timer in callback, it be handled at next loop

                    Take_LOCK(controller.lock);
                }

                // after updated, find out the minimum expires
                if (timerset[i].active && (minimum_expires > timerset[i].expires)) {
                    minimum_expires = timerset[i].expires;
                }
            }

            controller.in_task = 0;

            Give_LOCK(controller.lock);

            if ((ULLONG_MAX != minimum_expires) && (last_expires_set != minimum_expires)) {

                last_expires_set = minimum_expires;

                SERVICE_TIMER_DEBUG("set expires: %x\n", (uint32_t)minimum_expires);

                int ret = bsp_timer_set_expires_and_start(controller.bsp_timer_id, minimum_expires);
                if (0 != ret) {
                    syslog(LOG_ERR | LOG_COMP_SERVICES, "[Service timer] set expires and start error\n");
                }
            }
        }
    }
}

static int bsp_timer_callback(const void *arg)
{
    (void)arg;

    event_flag_signal_handle(controller.event, MASK_ISR, EVENT_FLAG_OR);

    return 0;
}

int service_timer_module_init(void)
{
    int ret;
    int error;

    if (1 == controller.module_inited) {
        error = -1;
        goto end;
    }

    memset(&controller, 0, sizeof(controller));
    memset(timerset, 0, sizeof(timerset));

    ret = event_flag_init_handle(&controller.event);
    if (0 != ret) {
        error = -2;
        goto end;
    }

    controller.lock = Create_LOCK(0);
    if(NULL == controller.lock) {
        error = -3;
        goto release_event;
    }

    // Why use one-shot mode ?
    // As we will 'always' change the timer's period
    controller.bsp_timer_id = bsp_init_timer(ULONG_MAX, bsp_timer_callback, NULL, 1);
    if (-1 == controller.bsp_timer_id) {
        error = -4;
        goto release_lock;
    }

    task_start("service timer", TASK_STACK_SIZE, TASK_PRIORITY, daemon, NULL);

    controller.module_inited = 1;

    return 0;

release_lock:
    // There is no function to release lock.
release_event:
    event_flag_destroy_handle(controller.event);
end:
    return error;
}

