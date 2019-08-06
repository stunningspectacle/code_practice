/*++
   INTEL CONFIDENTIAL
   Copyright (c) 2012 - 2017 Intel Corporation. All Rights Reserved.

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
 
#include <service_gpio.h>
#include <gpio.h>
#include <os_api.h>

#define configServices_GPIO_Max_CTX  (12)

#define ARRAY_SIZE(a)    (sizeof(a) / sizeof(*(a))) 

#define GET_NEXT_BY_PIN(_array, _pin, _from) \
({\
    int16_t i = _from;\
    int16_t size = ARRAY_SIZE(_array);\
    for (i = _from; i < size; i++) {\
        if(_array[i].valid && ((_array[i].registered_pin_map >> _pin) & 0x1)){\
            break;\
        }\
    }\
    if( i >= size){\
        i = -1;\
    }\
    i;\
})

#define OTHER_CONTEXT_HOLD_THIS_PIN(_array, _handle, _pin, _from) \
({\
    int16_t i = _from;\
    int16_t size = ARRAY_SIZE(_array);\
    for (i = _from; i < size; i++) {\
        if (_array[i].valid && ((_array[i].registered_pin_map >> _pin) & 0x1)){\
            if (i != _handle) {\
                break;\
            }\
        }\
    }\
    if( i >= size){\
        i = -1;\
    }\
    i;\
})

#define ID_IS_VALID(_array, _id) \
({\
    int ret;\
    if (!((_id) >= 0 && (_id) < ARRAY_SIZE(_array)) || _array[(_id)].valid == FALSE ) {\
        ret = FALSE;\
    } else {\
        ret = TRUE;\
    }\
    ret;\
})

#define CHECK_SERVICE \
({\
    if(service_gpio_started != TRUE ) { \
        syslog(LOG_ERR | LOG_COMP_SERVICES, "[GPIO] Service hasn't started!\n"); \
        return -1; \
    } \
})

#define CHECK_HANDLE(_handle) \
({\
    if ( ID_IS_VALID(service_gpio_handle_ctx, _handle) == FALSE) { \
        syslog(LOG_ERR | LOG_COMP_SERVICES, "[GPIO] Service handle %d is invalid!\n", _handle); \
        return -1;\
    }\
})

#define CHECK_PIN(_pin) \
({\
    if( _pin >= ISH_PIN_NUM) { \
        syslog(LOG_ERR | LOG_COMP_SERVICES, "[GPIO] pin[%d] not supported on this platform!\n", _pin); \
        return -1;\
    }\
})

#define CHECK_REGISTERED(_handle, _pin) \
({\
    if ( !((service_gpio_handle_ctx[handle].registered_pin_map >> pin) & 0x1) ) { \
        syslog(LOG_ERR| LOG_COMP_SERVICES, "[GPIO] pin[%d] hasn't registered for handle:%d\n", pin, handle); \
        return -1; \
    }\
})

typedef struct
{
    gpio_interrupt_mode_e int_mode;
    uint32_t event_mask;
    int enabled;
    bool (*top_half)(void);
    bool top_half_result;
} service_gpio_pin_conf_t;

typedef struct
{
    event_flag_handle_t event_flag; 
    service_gpio_pin_conf_t pins[ISH_PIN_NUM];
    uint32_t registered_pin_map;
    uint32_t gisr;
    int valid;  
} service_gpio_handle_ctx_t;


static int service_gpio_started = FALSE;
static LOCK_TYPE service_gpio_lock;
event_flag_handle_t event_flag_to_gpio_driver = 0;
static service_gpio_handle_ctx_t service_gpio_handle_ctx[configServices_GPIO_Max_CTX]; 

#ifdef DEBUG
void service_gpio_dump(void)
{
    int i;

    for (i=0; i<configServices_GPIO_Max_CTX; i++) {
        syslog(
                LOG_COMP_SERVICES | LOG_INFO,
                "event_flag=%x registered_pin_map=%x gisr=%u valid=%d\n",
                service_gpio_handle_ctx[i].event_flag,
                service_gpio_handle_ctx[i].registered_pin_map,
                service_gpio_handle_ctx[i].gisr,
                service_gpio_handle_ctx[i].valid);
    }
}
#endif

static char get_pin_level(char pin)
{
    gpio_wr_param_t gpio;

    gpio.dw = 0;
    gpio.pin = pin;
    gpio_read(&gpio);
    return gpio.value;
}

static void try_release_gpio(uint8_t pin)
{
    int loopcnt;
    bool loop_end = false;
    bool find_top_half = false;

   for (loopcnt = 0, loop_end = false; (loopcnt < 3) && !loop_end; loopcnt++) {
        for (int i = 0; i < configServices_GPIO_Max_CTX; i++) {
            if (service_gpio_handle_ctx[i].valid  &&
                    ((service_gpio_handle_ctx[i].registered_pin_map >> pin) & 0x1) &&
                    (service_gpio_handle_ctx[i].pins[pin].enabled)) {

                if (NULL != service_gpio_handle_ctx[i].pins[pin].top_half) {

                    find_top_half = true;

                    service_gpio_handle_ctx[i].pins[pin].top_half_result = 
                                            service_gpio_handle_ctx[i].pins[pin].top_half();

                    switch (service_gpio_handle_ctx[i].pins[pin].int_mode) {
                    case GPIO_INT_MODE_LOW_LEVEL_SW:
                    case GPIO_INT_MODE_FALLING_EDGE:
                        if (GPIO_ACTIVE_HIGH == get_pin_level(pin)) {
                            loop_end = true;
                        }
                        break;
                    case GPIO_INT_MODE_HIGH_LEVEL_SW:
                    case GPIO_INT_MODE_RISING_EDGE:
                        if (GPIO_ACTIVE_LOW == get_pin_level(pin)) {
                            loop_end = true;
                        }
                        break;
                    default:
                        DBG_ASSERT(0);
                    }
                }
            }
        }
    }

    if(find_top_half && (loopcnt >= 3)) {
        syslog(LOG_ALERT| LOG_COMP_SERVICES, "Failed to release pin[%d]!\n", pin);
    }
}

service_gpio_handle_t service_gpio_init_handle(event_flag_handle_t event_flag)
{
    int ctx_id = 0;

    CHECK_SERVICE;

    Take_LOCK(service_gpio_lock);
    
    ctx_id = FIND_FREE_ID(service_gpio_handle_ctx);
    if(ctx_id < 0) {
        syslog(LOG_ERR | LOG_COMP_SERVICES, "[GPIO] Get gpio service handle failed, max :%d\n", configServices_GPIO_Max_CTX);
        Give_LOCK(service_gpio_lock);
        return -1;
    }

    service_gpio_handle_ctx[ctx_id].event_flag = event_flag;
    service_gpio_handle_ctx[ctx_id].valid = TRUE;

    Give_LOCK(service_gpio_lock);

    return ctx_id;
}

int service_gpio_register_interrupt(service_gpio_handle_t handle, uint8_t pin,  gpio_interrupt_mode_e int_mode, uint32_t event_mask)
{
    bool shared_pin_error = FALSE;
    int other_ctx;

    CHECK_SERVICE;
    CHECK_HANDLE(handle);
    CHECK_PIN(pin);

    Take_LOCK(service_gpio_lock);

    /* check shared pin: allow falling edge and low level mode */
    other_ctx = OTHER_CONTEXT_HOLD_THIS_PIN(service_gpio_handle_ctx, handle, pin, 0);

    if (other_ctx >= 0) {
        syslog(LOG_DEBUG | LOG_COMP_SERVICES, "[Service GPIO] multi-context share pin\n");
        if ((service_gpio_handle_ctx[other_ctx].pins[pin].int_mode == int_mode) && ((GPIO_INT_MODE_FALLING_EDGE == int_mode))){
            shared_pin_error = FALSE;
        }
        else if ((service_gpio_handle_ctx[other_ctx].pins[pin].int_mode == int_mode) && ((GPIO_INT_MODE_LOW_LEVEL_SW == int_mode))) {
            shared_pin_error = FALSE;
        }
        else {
            shared_pin_error = TRUE;
        }
    }
    else {
        if ((service_gpio_handle_ctx[handle].registered_pin_map >> pin) & 0x1) {
            syslog(LOG_DEBUG | LOG_COMP_SERVICES, "[Service GPIO] update pin[%d] mode[%d]\n", pin, int_mode);
        }
        else {
            syslog(LOG_DEBUG | LOG_COMP_SERVICES, "[Service GPIO] pin[%d] first register as mode[%d]\n", pin, int_mode);
        }
    }

    if (shared_pin_error) {
        syslog(LOG_ERR | LOG_COMP_SERVICES, "[GPIO] pin[%d] share conflict, existed mode is %d, new mode is %d\n", 
            pin, service_gpio_handle_ctx[other_ctx].pins[pin].int_mode, int_mode);
        Give_LOCK(service_gpio_lock);
        return -1;
    }

    service_gpio_handle_ctx[handle].pins[pin].int_mode = int_mode;
    service_gpio_handle_ctx[handle].pins[pin].event_mask = event_mask;
    service_gpio_handle_ctx[handle].pins[pin].enabled = FALSE;
    service_gpio_handle_ctx[handle].registered_pin_map |= (0x1 << pin);
    
    Give_LOCK(service_gpio_lock);

    return 0;
}

int service_gpio_register_top_half(service_gpio_handle_t handle, uint8_t pin, bool (*top_half)(void))
{
    CHECK_SERVICE;
    CHECK_HANDLE(handle);
    CHECK_PIN(pin);

    if (NULL == top_half) {
        syslog(LOG_ERR| LOG_COMP_SERVICES, "[GPIO] top_half handler shouldn't be NULL!\n");
        return -1;
    }

    Take_LOCK(service_gpio_lock);

    service_gpio_handle_ctx[handle].pins[pin].top_half = top_half;

    Give_LOCK(service_gpio_lock);

    return 0;
}

int service_gpio_enable_interrupt(service_gpio_handle_t handle, uint8_t pin)
{
    int same_pin_id = 0;

    CHECK_SERVICE;
    CHECK_HANDLE(handle);
    CHECK_PIN(pin);
    CHECK_REGISTERED(handle, pin);

    if (service_gpio_handle_ctx[handle].pins[pin].enabled == TRUE) {
        syslog(LOG_WARNING | LOG_COMP_SERVICES, "[GPIO] pin[%d] interrupt already enabled for handle:%d\n", pin, handle);
        return 0;
    }

    Take_LOCK(service_gpio_lock);

    same_pin_id =  GET_NEXT_BY_PIN(service_gpio_handle_ctx, pin, 0);
    if(same_pin_id >=0 && service_gpio_handle_ctx[same_pin_id].pins[pin].enabled) {
        /* gpio interrupt already enabled in driver, no need config agian */
    } else {
        /* enable interrupt in gpio driver */

        gpio_ioctl_param_t gc;

        gc.pin = pin;
        gc.direction = GPIO_DIRECTION_INPUT;

        gc.enable_int = 1;
        gc.event_flag = event_flag_to_gpio_driver;
        gc.flag = GPIO_DRIVER_EVENT_INTERRUPT;

        if (service_gpio_handle_ctx[same_pin_id].pins[pin].int_mode == GPIO_INT_MODE_RISING_EDGE){
            gc.rising_edge = 1;
            gc.falling_edge = 0;
        } else {
            gc.rising_edge = 0;
            gc.falling_edge = 1;
        }

        gpio_ioctl(GPIO_IOCTL_SETUP, &gc);
    }

    service_gpio_handle_ctx[handle].pins[pin].enabled = TRUE;
    
    Give_LOCK(service_gpio_lock);

    return 0;
}

int service_gpio_disable_interrupt(service_gpio_handle_t handle, uint8_t pin)
{
    int same_pin_id = 0;

    CHECK_SERVICE;
    CHECK_HANDLE(handle);
    CHECK_PIN(pin);
    CHECK_REGISTERED(handle, pin);

    if (service_gpio_handle_ctx[handle].pins[pin].enabled == FALSE) {
        syslog(LOG_WARNING | LOG_COMP_SERVICES, "[GPIO] pin[%d] interrupt already disabled for handle:%d\n", pin, handle);
        return 0;
    }

    Take_LOCK(service_gpio_lock);

    same_pin_id =  GET_NEXT_BY_PIN(service_gpio_handle_ctx, pin, 0);
    if(same_pin_id >=0 && service_gpio_handle_ctx[same_pin_id].pins[pin].enabled) {
        /* others still used this interrupt, can't disable in driver */
    } else {
        /* disable interrupt in gpio driver */

        gpio_ioctl_param_t gc;

        gc.pin = pin;
        gc.direction = GPIO_DIRECTION_INPUT;

        gc.enable_int = 0;
        gc.event_flag = event_flag_to_gpio_driver;
        gc.flag = GPIO_DRIVER_EVENT_INTERRUPT;

        if (service_gpio_handle_ctx[same_pin_id].pins[pin].int_mode == GPIO_INT_MODE_RISING_EDGE){
            gc.rising_edge = 1;
            gc.falling_edge = 0;
        } else {
            gc.rising_edge = 0;
            gc.falling_edge = 1;
        }

        gpio_ioctl(GPIO_IOCTL_SETUP, &gc);
    }

    service_gpio_handle_ctx[handle].pins[pin].enabled = FALSE;
    
    Give_LOCK(service_gpio_lock);

    return 0;
}

int service_gpio_unregister_interrupt(service_gpio_handle_t handle, uint8_t pin)
{
    CHECK_SERVICE;
    CHECK_HANDLE(handle);
    CHECK_PIN(pin);

    if ( !((service_gpio_handle_ctx[handle].registered_pin_map >> pin) & 0x1) ) {
        syslog(LOG_WARNING | LOG_COMP_SERVICES, "[GPIO] pin[%d] hasn't registered for handle:%d\n", pin, handle);
        return 0;
    }

    service_gpio_disable_interrupt(handle, pin);

    Take_LOCK(service_gpio_lock);

    service_gpio_handle_ctx[handle].pins[pin].event_mask = 0;
    service_gpio_handle_ctx[handle].pins[pin].enabled = FALSE;
    service_gpio_handle_ctx[handle].pins[pin].top_half = NULL;
    service_gpio_handle_ctx[handle].pins[pin].top_half_result = false;
    service_gpio_handle_ctx[handle].registered_pin_map &= ~(1 << pin);

    Give_LOCK(service_gpio_lock);

    return 0;
}

int service_gpio_get_isr(service_gpio_handle_t handle, uint32_t* gisr, bool peek)
{
    CHECK_SERVICE;
    CHECK_HANDLE(handle);

    if(gisr != NULL) {
        *gisr = service_gpio_handle_ctx[handle].gisr;
		if (!peek)
			service_gpio_handle_ctx[handle].gisr = 0;
    }

    return 0;
}

static void service_gpio_task(void * param)
{
    (void)param; 

    int result = 0;
    unsigned int event_mask = GPIO_DRIVER_EVENT_INTERRUPT;
    unsigned int actual_flags = 0;
    uint32_t g_isr = 0;
    uint8_t pin = 0;
    int i = 0;
    
    while(true) {
        result = event_flag_reltimedwait_handle(event_flag_to_gpio_driver, event_mask, EVENT_FLAG_OR_CLEAR, &actual_flags, -1);
        if (result != 0) {
            syslog(LOG_WARNING | LOG_COMP_SERVICES, "[GPIO] failed waiting on event flag. status %d\n", result);
            continue;
        }
        
        if (actual_flags & GPIO_DRIVER_EVENT_INTERRUPT) {
            /* got a interrupt from GPIO driver */
            gpio_wr_param_t param;
            gpio_getisr(&param, false);    
            g_isr = param.dw;

            for(pin = 0; pin < ISH_PIN_NUM; pin++) {
                if((g_isr >> pin) & 0x1 ) {

                    try_release_gpio(pin);
                   
                    for(i = 0; i < configServices_GPIO_Max_CTX; i++) {
                        if(service_gpio_handle_ctx[i].valid  && (service_gpio_handle_ctx[i].registered_pin_map >> pin) & 0x1 &&
                            service_gpio_handle_ctx[i].pins[pin].enabled ) {
                            service_gpio_handle_ctx[i].gisr |= g_isr;
                            if ((NULL != service_gpio_handle_ctx[i].pins[pin].top_half) &&
                                    (false == service_gpio_handle_ctx[i].pins[pin].top_half_result)) {
                                syslog(LOG_DEBUG | LOG_COMP_SERVICES, "[GPIO] no need to send signal\n");
                            }
                            else {
                                event_flag_signal_handle(service_gpio_handle_ctx[i].event_flag, service_gpio_handle_ctx[i].pins[pin].event_mask, EVENT_FLAG_OR);
                            }
                        }
                    }
                }
            }
        }
    }
}

void service_gpio_start(void)
{
    if(service_gpio_started == TRUE ) {
        syslog(LOG_WARNING| LOG_COMP_SERVICES, "[GPIO] gpio service already started!\n");
        return ;
    }

    if(event_flag_init_handle(&event_flag_to_gpio_driver) != SUCCESS) {
        syslog(LOG_ERR | LOG_COMP_SERVICES, "[GPIO] can't initialize event for gpio service!\n");
        return;
    }

    memset(&service_gpio_handle_ctx, 0, sizeof(service_gpio_handle_ctx));
    
    service_gpio_lock = Create_LOCK(0);
    
    task_start("service_gpio", 1200, 1, service_gpio_task, NULL);

    service_gpio_started = TRUE;
}

