/*
 * Copyright (C) 2011-2014 Intel Mobile Communications GmbH
 * Copyright (C) 2010 ARM Limited. All rights reserved.
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

#include <limits.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cutils/log.h>
#include <cutils/atomic.h>

#include <hardware/hardware.h>
#include <hardware/gralloc.h>
#include <ion/ion.h>

#include "gralloc_priv.h"
#include "gr.h"

/*****************************************************************************/

static int gralloc_map(gralloc_module_t const* module, buffer_handle_t handle)
{
    private_handle_t* hnd = (private_handle_t*)handle;

    if((hnd->usage & GRALLOC_USAGE_PROTECTED) == 0) {
        void* mappedAddress = mmap(0, hnd->size, PROT_READ|PROT_WRITE, MAP_SHARED,
            hnd->fd, 0);
        if (mappedAddress == MAP_FAILED) {
            ALOGE("%s: could not mmap %s", __func__, strerror(errno));
            return -errno;
        }
        ALOGV("%s: base %p %d %d %d\n", __func__, mappedAddress, hnd->size,
            hnd->w, hnd->h);
        hnd->base = intptr_t(mappedAddress);
    } else {
        ALOGV("gralloc_map() secure buffer; don't map");
        hnd->base = 0;
    }
    return 0;
}

static int gralloc_unmap(gralloc_module_t const* module, buffer_handle_t handle)
{
    private_handle_t* hnd = (private_handle_t*)handle;

    if (!hnd->base)
        return 0;

    if (munmap((void*)hnd->base, hnd->size) < 0) {
        ALOGE("%s :could not unmap %s 0x%x %d", __func__, strerror(errno),
              hnd->base, hnd->size);
    }
    ALOGV("%s: base 0x%x %d %d %d\n", __func__, hnd->base, hnd->size,
          hnd->w, hnd->h);
    hnd->base = 0;
    return 0;
}


static int gralloc_terminate(gralloc_module_t const* module,
        buffer_handle_t handle)
{
	int err=0;
    private_handle_t* hnd = (private_handle_t*)handle;
    if (!(hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER)) {
        void* base = (void*)hnd->base;
        size_t size = hnd->size;
        ALOGV("%s from %p, size=%d", __FUNCTION__, base, size);
        if (munmap(base, size) < 0) {
            ALOGE("%s Could not unmap %s",__FUNCTION__, strerror(errno));
        }
    }
    hnd->base = 0;
    return 0;
}
/*****************************************************************************/

static pthread_mutex_t s_map_lock = PTHREAD_MUTEX_INITIALIZER;

int grallocUnmap(gralloc_module_t const* module, private_handle_t *hnd)
{
    return gralloc_unmap(module, hnd);
}

int getIonFd(gralloc_module_t const *module)
{
    private_module_t* m = const_cast<private_module_t*>(reinterpret_cast<const private_module_t*>(module));
    if (m->ion_client == -1)
        m->ion_client = ion_open();
    return m->ion_client;
}

/*****************************************************************************/
int gralloc_getphysaddr(struct gralloc_module_t const* module,
		buffer_handle_t handle, unsigned int *PhysAddr)
{
	if (private_handle_t::validate(handle) < 0)
		return -EINVAL;

	int err = 0;
	private_handle_t* hnd = (private_handle_t*)handle;
	print_private_handle(hnd, "gralloc_getphysaddr" );
	*PhysAddr = hnd->phys;
	return err;
}

int gralloc_register_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
 	if (private_handle_t::validate(handle) < 0)
	{
		ALOGE("Registering invalid buffer 0x%x, returning error", (int)handle);
		return -EINVAL;
	}

	private_handle_t* hnd = (private_handle_t*)handle;

	int retval = -EINVAL;

  print_private_handle(hnd, "gralloc_register_buffer" );

  if(hnd->usage & GRALLOC_USAGE_PROTECTED) {
      ALOGV("gralloc_register_buffer() protected buffer, don't register 0x%x", (int)handle);
      return 0;
  }

	pthread_mutex_lock(&s_map_lock);

    if (hnd->flags & private_handle_t::PRIV_FLAGS_FRAMEBUFFER){
	    ALOGE( "Can't register buffer 0x%x as it is a framebuffer", (unsigned int)handle );
    }else if ( hnd->flags & private_handle_t::PRIV_FLAGS_CONTIGUOUS ){
	    int ret;
	    unsigned char *mappedAddress;
	    size_t size = hnd->size;

	    ret = ion_import(getIonFd(module), hnd->fd, &hnd->ion_hdl);
	    if (ret)
		    ALOGE("error importing handle %d\n", hnd->fd);

#if 0
	    ret = gralloc_map(module, handle);
	    if (ret)
		    ALOGE("error mapping handle %d\n", hnd->fd);
#else
	    mappedAddress = (unsigned char*)mmap( 0, size, PROT_READ | PROT_WRITE, MAP_SHARED, hnd->fd, 0 );

	    if ( MAP_FAILED == mappedAddress ) {
		    ALOGE( "mmap( fd:%d ) failed with %s",  hnd->fd, strerror( errno ) );
		    print_private_handle(hnd, "mmap failed" );
		    retval = -errno;
		    goto cleanup;
	    }
#endif

	    hnd->base = intptr_t(mappedAddress) + hnd->offset;
	    ALOGV("%s: fd: %d, phys: 0x%x, virt:0x%08x\n", __FUNCTION__, hnd->fd, hnd->phys, hnd->base);
	    pthread_mutex_unlock( &s_map_lock );
	    return 0;
    } else {
	    int ret;
	    unsigned char *mappedAddress;
	    size_t size = hnd->size;

       mappedAddress = (unsigned char*)mmap( 0, size, PROT_READ, MAP_SHARED, hnd->fd, 0 );
  
       if ( MAP_FAILED == mappedAddress ) {
          ALOGE( "mmap( fd:%d ) failed with %s",  hnd->fd, strerror( errno ) );
          print_private_handle(hnd, "mmap failed" );
          retval = -errno;
          goto cleanup;
       }
  
       hnd->base = intptr_t(mappedAddress) + hnd->offset;
       ALOGV("%s: fd: %d, phys: 0x%x, virt:0x%08x\n", __FUNCTION__, hnd->fd, hnd->phys, hnd->base);
       pthread_mutex_unlock( &s_map_lock );
       return 0;
    }

cleanup:
	pthread_mutex_unlock(&s_map_lock);
    return retval;
}

int gralloc_unregister_buffer(gralloc_module_t const* module,
        buffer_handle_t handle)
{
    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    ALOGV("%s: base 0x%08x %d %d %d\n", __func__, hnd->base, hnd->size,
          hnd->w, hnd->h);

    print_private_handle((private_handle_t*)hnd, "gralloc_unregister_buffer" );

    if(hnd->usage & GRALLOC_USAGE_PROTECTED) {
        ALOGV("gralloc_unregister_buffer() protected buffer, don't unregister");
        return 0;
    }

    void* base = (void*)hnd->base;
    size_t size = hnd->size;

    if ( munmap( base,size ) < 0 )
    {
	    ALOGE("Could not munmap base:0x%x size:%d '%s'",
			    (unsigned int)base, size, strerror(errno));
    }

    if (hnd->ion_hdl)
        ion_free(getIonFd(module), hnd->ion_hdl);

    ALOGV("%s", __FUNCTION__);


	return 0;
}

int terminateBuffer(gralloc_module_t const* module,
        private_handle_t* hnd)
{
    if (hnd->base) {
        // this buffer was mapped, unmap it now
        gralloc_terminate(module, hnd);
    }

    return 0;
}
int gralloc_lock(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        void** vaddr)
{
    // this is called when a buffer is being locked for software
    // access. in thin implementation we have nothing to do since
    // not synchronization with the h/w is needed.
    // typically this is used to wait for the h/w to finish with
    // this buffer if relevant. the data cache may need to be
    // flushed or invalidated depending on the usage bits and the
    // hardware.

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    if (!hnd->base)
        gralloc_map(module, hnd);
    if (vaddr != NULL)
        *vaddr = (void*)hnd->base;
    return 0;
}

int gralloc_flush_cache(gralloc_module_t const* module, buffer_handle_t handle);

int gralloc_lock_ycbcr(gralloc_module_t const* module,
        buffer_handle_t handle, int usage,
        int l, int t, int w, int h,
        struct android_ycbcr *ycbcr)
{
    // this is called when a buffer is being locked for software
    // access. in thin implementation we only fill ycbcr since
    // not synchronization with the h/w is needed.
    // typically this is used to wait for the h/w to finish with
    // this buffer if relevant. the data cache may need to be
    // flushed or invalidated depending on the usage bits and the
    // hardware.

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;

    private_handle_t* hnd = (private_handle_t*)handle;
    if (!hnd->base)
        gralloc_map(module, hnd);

    // this is currently only used by camera for yuv420sp
    // if in future other formats are needed, store to private
    // handle and change the below code based on private format.
    int ystride = hnd->byte_stride;
    ycbcr->y  = (void*)hnd->base;
    ycbcr->cr = (void*)(hnd->base + ystride * hnd->h);
    ycbcr->cb = (void*)(hnd->base + ystride * hnd->h + 1);
    ycbcr->ystride = ystride;
    ycbcr->cstride = ystride;
    ycbcr->chroma_step = 2;
    memset(ycbcr->reserved, 0, sizeof(ycbcr->reserved));

    return 0;
}

int gralloc_unlock(gralloc_module_t const* module, buffer_handle_t handle)
{
    // we're done with a software buffer. nothing to do in this
    // implementation. typically this is used to flush the data cache.
#if 0
    private_handle_t* hnd = (private_handle_t*)handle;
    ion_sync_fd(getIonFd(module), hnd->fd);

    if (private_handle_t::validate(handle) < 0)
        return -EINVAL;
#endif
    return gralloc_flush_cache(module, handle);
}

int gralloc_flush_cache(gralloc_module_t const* module, buffer_handle_t handle)
{
    private_handle_t* hnd = (private_handle_t*)handle;
    int retval = 0;
    int usage = 0;
    pthread_mutex_lock(&s_map_lock);
    usage = hnd->usage;
    if ((usage & GRALLOC_USAGE_HW_MASK) && (usage & GRALLOC_USAGE_PRIVATE_3)) {
        retval = ion_sync_fd(getIonFd(module), hnd->fd);
        if (retval)
            ALOGE("error cache flush of fd %d, hnd->ion_hdl 0x%x, ion_fd 0x%x\n", hnd->fd, hnd->ion_hdl, hnd->ion_fd);
    }
    pthread_mutex_unlock(&s_map_lock);
    return retval;
}
