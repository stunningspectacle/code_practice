/*
 * Copyright (C) 2011-2013 Intel Mobile Communications GmbH
 *
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GR_H_
#define GR_H_

#include <stdint.h>
#ifdef HAVE_ANDROID_OS      // just want PAGE_SIZE define
# include <asm/page.h>
#else
# include <sys/user.h>
#endif
#include <limits.h>
#include <sys/cdefs.h>
#include <hardware/gralloc.h>
#include <pthread.h>
#include <errno.h>

#include <cutils/native_handle.h>

/*****************************************************************************/

struct private_module_t;
struct private_handle_t;

static inline void print_private_handle(struct private_handle_t* hnd, const char* str)
{
	ALOGV("%s [fd:%d flags:0x%x size:%d %dx%d offset:0x%x base:0x%08x phys:0x%08x fmt:0x%x ion{hdl:%p}]",
		str, hnd->fd, hnd->flags, hnd->size, hnd->w, hnd->h, hnd->offset,
		hnd->base, hnd->phys, hnd->fmt, (void*)hnd->ion_hdl);
}


inline size_t roundUpToPageSize(size_t x) {
    return (x + (PAGE_SIZE-1)) & ~(PAGE_SIZE-1);
}

int init_frame_buffer_locked(alloc_device_t* dev, struct private_module_t* module);
int gralloc_alloc_hwbuffer(alloc_device_t* dev, size_t size, int usage, buffer_handle_t* pHandle, int flags);
int terminateBuffer(gralloc_module_t const* module, private_handle_t* hnd);

int grallocUnmap(gralloc_module_t const* module, private_handle_t *hnd);

/*****************************************************************************/

class Locker {
    pthread_mutex_t mutex;
public:
    class Autolock {
        Locker& locker;
    public:
        inline Autolock(Locker& locker) : locker(locker) {  locker.lock(); }
        inline ~Autolock() { locker.unlock(); }
    };
    inline Locker()        { pthread_mutex_init(&mutex, 0); }
    inline ~Locker()       { pthread_mutex_destroy(&mutex); }
    inline void lock()     { pthread_mutex_lock(&mutex); }
    inline void unlock()   { pthread_mutex_unlock(&mutex); }
};

#endif /* GR_H_ */
