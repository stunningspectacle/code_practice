/*
 * Copyright (C) 2013 Intel Mobile Communications GmbH
 *
 * Copyright (C) 2010 The Android Open Source Project
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
 *
 * Jan 16 2013: Create minimum working hwcomposer
 *
 */
//#define LOG_NDEBUG 0
#include <fcntl.h>
#include <errno.h>
#include <poll.h>

#include <sys/types.h>

#include <utils/CallStack.h>
#include <utils/threads.h>
#include <utils/Vector.h>

#include <cutils/log.h>
#include <cutils/atomic.h>
#include <cutils/properties.h>

#include <hardware/hardware.h>
#include <hardware/hwcomposer.h>
#include <hardware/gralloc.h>
#include <video/xgold-dcc.h>

#include <EGL/egl.h>

#include "../libgralloc/gralloc_priv.h"
#include "dcc-hal.h"

#include "hwcomposer_video.h"

#define BUF_DUMP_FILE_PATTERN "/data/hwcdump-%d.raw"
#define BUF_DUMP_FILE_PATTERN_OVR "/data/hwcdump-%d-ovr.raw"

/*****************************************************************************/
extern "C" int clock_nanosleep(clockid_t clock_id, int flags,
                           const struct timespec *request,
                           struct timespec *remain);

namespace android {

/* VSYNC */
class IntelVsyncThread : public Thread {
public:
	IntelVsyncThread(const char* threadName,
			struct hwc_context_t *pdev) :
			name(threadName), p_hwc_context(pdev), mEnabled(false) {
	}
	virtual void setEnabled(bool enabled) = 0;

	const char* name;
protected:
	struct hwc_context_t *p_hwc_context;
	mutable Mutex mLock;
	Condition mCondition;
	bool mEnabled;
};

class hwcVsyncThread : public IntelVsyncThread {
	int vsync_fd;
	struct pollfd poll_fds[1];
	virtual void onFirstRef();
	virtual bool threadLoop();
	public:
		hwcVsyncThread(struct hwc_context_t *p_hwc_context);
		~hwcVsyncThread();
		void setEnabled(bool enabled);
};

class hwcFakeVsyncThread : public IntelVsyncThread {
	mutable nsecs_t mNextFakeVsync;
	nsecs_t mRefreshPeriod;
	virtual void onFirstRef();
	virtual bool threadLoop();
	public:
		hwcFakeVsyncThread(struct hwc_context_t *p_hwc_context);
		void setEnabled(bool enabled);
};

static int to_bpp_hal_fmt(int hal_pix_fmt);

enum {
    DCC_FMT_YUV422PACKED    = 1,
    DCC_FMT_YUV420PLANAR    = 2,
    DCC_FMT_YVU420PLANAR    = 3,
    DCC_FMT_YUV422PLANAR    = 4,
    DCC_FMT_RGB565      = 5,
    DCC_FMT_RGB888      = 6,
    DCC_FMT_RGB4444     = 7,
    DCC_FMT_RGB1555     = 8,
    DCC_FMT_ARGB8888    = 9,
    DCC_FMT_BGR565      = 10,
    DCC_FMT_BGR888      = 11,
    DCC_FMT_ABGR8888    = 12,
    DCC_FMT_YUV444PACKED    = 13,
    DCC_FMT_YUV444PLANAR    = 14,
    DCC_FMT_YUV444SP    = 15,
    DCC_FMT_YUV422SP    = 16,
    DCC_FMT_YUV420SP    = 17,
};

static const char* hwc_comp2str(int compositionType) {
    switch (compositionType) {
    case HWC_FRAMEBUFFER:
        return "FB";
    case HWC_OVERLAY:
        return "OV";
    case HWC_BACKGROUND:
        return "BG";
    case HWC_FRAMEBUFFER_TARGET:
        return "FT";
    default:
        break;
    }
    return "??";
}

static const char* hwc_blend2str(int blending) {
    switch (blending) {
    case HWC_BLENDING_NONE:
        return "NO";
    case HWC_BLENDING_PREMULT:
        return "PM";
    case HWC_BLENDING_COVERAGE:
        return "CO";
    default:
        break;
    }
    return "??";
}

static const char* hwc_fmt2str(int fmt) {
    switch (fmt) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
        return "RGBA_8888";
    case HAL_PIXEL_FORMAT_RGBX_8888:
        return "RGBX_8888";
    case HAL_PIXEL_FORMAT_RGB_888:
        return " RGB_888 ";
    case HAL_PIXEL_FORMAT_RGB_565:
        return " RGB_565 ";
    case HAL_PIXEL_FORMAT_BGRA_8888:
        return "BGRA_8888";
    default:
        break;
    }
    static char f[32];
    snprintf(&f[0], 32, "%9d", fmt);
    return &f[0];
}

static void hwc_dump_layer(hwc_layer_1_t const* l, int i) {
    int format = -1;
    int flags = 0;
    int stride = -1;

    if (l->handle) {
        private_handle_t const * h =
                reinterpret_cast<private_handle_t const*>(l->handle);
        format = h->fmt;
        flags =  h->flags;
    }

    ALOGV(
            "\t%d type=%s, flags=%08x, handle=%p, tr=%02x, blend=%s, [%d,%d,%d,%d : %d], {%d,%d,%d,%d}, fmt=%s, flags=%x",
            i, hwc_comp2str(l->compositionType), l->flags, l->handle,
            l->transform, hwc_blend2str(l->blending), l->sourceCrop.left,
            l->sourceCrop.top, l->sourceCrop.right, l->sourceCrop.bottom, stride,
            l->displayFrame.left, l->displayFrame.top, l->displayFrame.right,
            l->displayFrame.bottom, hwc_fmt2str(format), flags);
}

static void hwc_dump_buffer(hwc_layer_1_t const* l, const char* filename, int i) {
	static FILE* buf_dump_file = 0;

	ALOGD("hwc_dump_buffer [%p]", l->handle);
	if (l->handle) {
		private_handle_t const * h;
		h = reinterpret_cast<private_handle_t const*>(l->handle);
		if (buf_dump_file == 0) {
			ALOGV("opening buffer dump file");
			buf_dump_file = fopen(filename, "w");
			ALOGV("open returned %p", buf_dump_file);
		}
		if (buf_dump_file != 0) {
			void *buffer_addr;
			buffer_addr = reinterpret_cast<void*>(h->base);
			ALOGD("dump buffer id=%d, base=%8x, addr=%p, length=%d to %s\n",
					i, h->base, buffer_addr, h->size, filename);
			fwrite(buffer_addr, 1, h->size, buf_dump_file);
			fflush(buf_dump_file);
			fclose(buf_dump_file);
			buf_dump_file = 0;
		} else {
			ALOGE("can not dump buffer: buf_dump_fd=%p", buf_dump_file);
		}
	}

}

class LayerConfiguration {
public:

    enum AddOvResult {
        ADDOV_RES_OK = 0,
        ADDOV_RES_NO_HANDLE,
        ADDOV_RES_TOO_MANY,
        ADDOV_RES_CANT_TRANSFORM,
        ADDOV_RES_WRONG_FMT,
        ADDOV_RES_CANT_SCALE,
        ADDOV_RES_NOT_FULLSCREEN,
        ADDOV_RES_SKIP_LAYER,
    };

    LayerConfiguration() :
        configuratorName(DEFAULT_CONFIGURATOR_NAME),
        fbt_index(0),
        fbt_enabled(true),
        back(0),
        backIndex(0),
        overlay_count(0),
        fb_count(0) {
        for (size_t i = 0; i < HWC_OVERLAY_NUM; i++)
            overrides[i] = 0;
    }

    const char* configuratorName;

    size_t  fbt_index; // index of the framebuffer target layer
    bool    fbt_enabled; // true if the framebuffer target should be displayed
    size_t  ppLayerIndex; // index of the layer that requires post-processing

    // DCC background frame buffer
    hwc_layer_1_t* back;
    size_t  backIndex;

    // DCC overlays
    size_t  overlay_count;
    hwc_layer_1_t* overlays[HWC_OVERLAY_NUM];
    hwc_layer_1_t* overrides[HWC_OVERLAY_NUM];
    size_t         overlayIndexes[HWC_OVERLAY_NUM];

    size_t  fb_count;
    hwc_layer_1_t* fbLayers[HWC_MAX_LAYER_NUM];
    buffer_handle_t fbLastHandle[HWC_MAX_LAYER_NUM];

    static const char *DEFAULT_CONFIGURATOR_NAME;

    void reset(hwc_display_contents_1_t* display) {
        configuratorName = DEFAULT_CONFIGURATOR_NAME;
        overlay_count = 0;
        fb_count = 0;
        memset(fbLastHandle, 0, sizeof(fbLastHandle));
        fbt_enabled = false;
        ppLayerIndex = HWC_OVERLAY_INVALID;
        // find the composition target buffer and reset
        // all HWC_OVERLAY back to be HWC_FRAMEBUFFER
        for (size_t i = 0; i < display->numHwLayers; i++) {
            if (display->hwLayers[i].compositionType
                    == HWC_FRAMEBUFFER_TARGET) {
                fbt_index = i;
                fbt_enabled = true;
                back = &display->hwLayers[i];
                backIndex = fbt_index;
            }

            if (display->hwLayers[i].compositionType == HWC_OVERLAY) {
                display->hwLayers[i].compositionType = HWC_FRAMEBUFFER;
            }
        }
        for (size_t i = 0; i < HWC_OVERLAY_NUM; i++) {
            // do not delete the override layers
            // ownership stays with their producer
            overrides[i] = 0;
        }
    }

    AddOvResult addOverlay(hwc_display_contents_1_t* display, size_t hwc_index,
            bool allowScalingAndRotate = false, bool allowYCbCr = false) {
        size_t count = display->numHwLayers;
        hwc_layer_1_t* l = &display->hwLayers[hwc_index];
        private_handle_t const * h =
                reinterpret_cast<private_handle_t const*>(l->handle);
        // check that this layer has a valid handle
        if (h == 0)
            return ADDOV_RES_NO_HANDLE;
        // if count > 3 then we have at least 3 layers other
        // than the FBT, we can not render that with two overlays in any case
        // so reserve one overlay for the FBT itself (limit overlay_count to 1)
        if (count > 3 && overlay_count >= 1)
            return ADDOV_RES_TOO_MANY;
        // if count is 3 we could render all layers using overlays
        // allow 2 layers as no FBT is needed for sure
        if (count == 3 && overlay_count > 1)
            return ADDOV_RES_TOO_MANY;
        // bypass HWC_SKIP_LAYER layer
        if (l->flags & HWC_SKIP_LAYER)
            return ADDOV_RES_SKIP_LAYER;
        // no rotation
        if (!allowScalingAndRotate && l->transform)
            return ADDOV_RES_CANT_TRANSFORM;
        // check color format
        if (!isFormatSupportedAsOverlay(h->fmt, allowYCbCr))
            return ADDOV_RES_WRONG_FMT;
        // no scaling for now
        bool sameSize = compareSizes(l->displayFrame, l->sourceCrop);
        if (!allowScalingAndRotate && !sameSize)
            return ADDOV_RES_CANT_SCALE;
        // only first overlay can have arbitrary size
        if (overlay_count > 0 && !compareSizes(l->displayFrame, display->hwLayers[fbt_index].sourceCrop))
            return ADDOV_RES_NOT_FULLSCREEN;

        // if the layer needs resize/rotation then it has to be post-processed
        if (!sameSize || l->transform)
            ppLayerIndex = hwc_index;

        overlayIndexes[overlay_count] = hwc_index;
        overlays[overlay_count] = l;
        overlay_count++;
        // disable back as soon as we have one overlay
        back = 0;
        return ADDOV_RES_OK;
    }

    AddOvResult addFBTAsOverlay(hwc_display_contents_1_t* display) {
        size_t count = display->numHwLayers;
        hwc_layer_1_t* l = &display->hwLayers[fbt_index];
        private_handle_t const * h =
                reinterpret_cast<private_handle_t const*>(l->handle);
        // check that this layer has a valid handle
        if (h == 0)
            return ADDOV_RES_NO_HANDLE;
        // all overlays already used
        if (overlay_count > 1)
            return ADDOV_RES_TOO_MANY;
        // no rotation
        if (!l->transform == 0)
            return ADDOV_RES_CANT_TRANSFORM;
        // check color format
        if (!isFormatSupportedAsOverlay(h->fmt))
            return ADDOV_RES_WRONG_FMT;
        // no scaling for now
        if (!compareSizes(l->displayFrame, l->sourceCrop))
            return ADDOV_RES_CANT_SCALE;

        overlayIndexes[overlay_count] = fbt_index;
        overlays[overlay_count] = l;
        overlay_count++;
        // disable back as soon as we have one overlay
        back = 0;
        return ADDOV_RES_OK;
    }

    bool compareRects(const hwc_rect_t& r1, const hwc_rect_t& r2) {
        return r1.top == r2.top && r1.left == r2.left && r1.right == r2.right
                && r1.bottom == r2.bottom;
    }

    bool compareSizes(const hwc_rect_t& r1, const hwc_rect_t& r2) {
        return (r1.bottom - r1.top) == (r2.bottom - r2.top);
    }

    bool isFormatSupportedAsOverlay(int hal_pix_fmt, bool allowYCbCr = false) {
        switch (hal_pix_fmt) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
        case HAL_PIXEL_FORMAT_RGB_565:
            return true;
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
            return allowYCbCr;
        default:
            return false;
        }
    }

    hwc_layer_1_t* getAt(size_t index) {
        hwc_layer_1_t* l = 0;
        if (index < overlay_count) {
            l = overrides[index];
            if (l == 0)
                l = overlays[index];
        }
        return l;
    }
};

const char *LayerConfiguration::DEFAULT_CONFIGURATOR_NAME = "none";

/**
 * Base class for all configurators.
 * It defines a configure operation that returns true
 * if the configurator could correctly configure an update
 * for the given layer structure.
 */
class IConfigurator {
public:
    const char *name;
    IConfigurator(const char *n) : name(n) {}
    virtual ~IConfigurator() {}
    virtual bool configure(hwc_display_contents_1_t* display, LayerConfiguration& conf) {
        (void) display;
        (void) conf;
        return false;
    }

protected:

    private_handle_t const * getHandle(hwc_layer_1_t* l) {
        return reinterpret_cast<private_handle_t const*>(l->handle);
    }
};
/**
 * This configurator can render a display that has:
 * - a single layer
 * - same size as the FBT (i.e. fullscreen)
 */
class SingleLayerConfigurator : public IConfigurator {
public:
    SingleLayerConfigurator() : IConfigurator("single") {}
    bool configure(hwc_display_contents_1_t* display, LayerConfiguration& conf) {
        // 1 layer only (plus FBT)
        if (display->numHwLayers != 2)
            return false;
        hwc_layer_1_t* l = &display->hwLayers[0];
        hwc_layer_1_t* fbt_l = &display->hwLayers[conf.fbt_index];
        // no scaling/offsets
        if (!conf.compareRects(l->displayFrame, l->sourceCrop))
            return false;
        // same size as FBT
        if (!conf.compareRects(l->displayFrame, fbt_l->displayFrame))
            return false;

        return conf.addOverlay(display, 0) == LayerConfiguration::ADDOV_RES_OK;
    }
};

/**
 * This configurator can render a display that has:
 * - a video layer (currently identified by the RGB565 format)
 * - if only another layer is present it is rendered using a second overlay
 * - otherwise their composition (done by SF) is rendered on the second overlay
 */
class VideoOverlayConfigurator : public IConfigurator {
public:
    VideoOverlayConfigurator() : IConfigurator("video") {}
    bool addProtectedVideo(hwc_display_contents_1_t* display, LayerConfiguration& conf, size_t& video_id) {
        bool canDo = false;
        for (video_id = 0; video_id < display->numHwLayers; video_id++) {
            hwc_layer_1_t* l = &display->hwLayers[video_id];
            private_handle_t const *h = getHandle(l);
            if ((h != 0) && (h->usage & GRALLOC_USAGE_PROTECTED)) {
                // protected video buffer, can only be rendered
                // using HW composition.
                LayerConfiguration::AddOvResult res;
                res = conf.addOverlay(display, video_id, true, true);
                canDo = res == LayerConfiguration::ADDOV_RES_OK;
                if(!canDo)
                    ALOGE("Cannot render protected layer using HW: err=%d", res);
                break;
            }
        }
        return canDo;
    }
    bool addVideo(hwc_display_contents_1_t* display, LayerConfiguration& conf, size_t& video_id) {
        bool canDo = false;
        for (video_id = 0; video_id < display->numHwLayers; video_id++) {
            if (video_id == conf.fbt_index)
                continue;
            hwc_layer_1_t* l = &display->hwLayers[video_id];
            if (isVideoLayer(l)) {
                // video buffer, can be rendered using HW composition.
                LayerConfiguration::AddOvResult res;
                res = conf.addOverlay(display, video_id, false, false);
                canDo = res == LayerConfiguration::ADDOV_RES_OK;
                break;
            }
        }
        return canDo;
    }
    bool configure(hwc_display_contents_1_t* display, LayerConfiguration& conf) {
        size_t video_id;
        LayerConfiguration::AddOvResult res;
        bool canDo = false;
        // search first for a protected video layer, then for a normal video layer
        canDo = addProtectedVideo(display, conf, video_id) ||
                addVideo(display, conf, video_id);

        // manage the other layers
        if (canDo && display->numHwLayers > 2) {
            // there are other layers
            if (display->numHwLayers == 3) {
                // there is only another layer, try to render it as overlay,
                // the index can only be 0 or 1
                int other_id = video_id == 0 ? 1 : 0;
                res = conf.addOverlay(display, other_id);
                canDo = res == LayerConfiguration::ADDOV_RES_OK;
            } else {
                canDo = false;
            }
            if (!canDo) {
                // last chance: add FBT as second overlay,
                // if this fails we give up this configuration
                res = conf.addFBTAsOverlay(display);
                canDo = (res == LayerConfiguration::ADDOV_RES_OK);
                if (!canDo) {
                    ALOGE("Cannot add FBT (index=%d), error=%d", conf.fbt_index, res);
                }
            }
        }
exit:
        return canDo;
    }

    bool isVideoLayer(hwc_layer_1_t* l) {
        private_handle_t const *h = getHandle(l);
        // check for the proprietary YCbCr420SP format
        if ((h != 0) && (h->fmt == HAL_PIXEL_FORMAT_YCbCr_420_SP))
            return true;
        return false;
    }

};

/**
 * Generic overlay configurator.
 * This configurator iterates through the layers and try to render
 * as many as possible using HW overlays.
 */
class GenOverlayConfigurator : public IConfigurator {
public:
    GenOverlayConfigurator() : IConfigurator("gen-overlay") {}
    bool configure(hwc_display_contents_1_t* display, LayerConfiguration& conf) {
        // try to set up overlays
        for (size_t i = 0; i < display->numHwLayers; i++) {
            // ignore the composition target buffer
            if (display->hwLayers[i].compositionType
                    == HWC_FRAMEBUFFER_TARGET) {
                continue;
            }
            if (conf.addOverlay(display, i) != LayerConfiguration::ADDOV_RES_OK) {
                ALOGV("Could not add overlay for layer %d", i);
                hwc_dump_layer(&display->hwLayers[conf.overlayIndexes[i]], i);
                // It is not possible to render all layers using HW overlay.
                // Let SurfaceFlinger compose the topmost surfaces into the
                // frame buffer target and use HW to blend it as an overlay.
                break;
            }
        }
        // check if we need the FBT
        if (display->numHwLayers > 1 &&
                conf.overlay_count + 1 == display->numHwLayers) {
            // all layers (except for the framebuffer) can be
            // rendered using overlays -> disable framebuffer target
            conf.fbt_enabled = false;
        }
        return true;
    }
};

class Updater {

    enum {
        UPDATE_STAT_NUM = 64
    };

    enum {
        CONFIG_COUNTERS_NUM = 10
    };

    enum {
        HWC_COMPOSITOR_NUM = 3
    };

public:
    enum {
        HWC_STAT_CONF_NUM = 10
    };

    LayerConfiguration conf;
    size_t  configure_count;
    bool    disable_hw_composition;
    bool	dump_buffers;
    bool    dump_fbt;
    IConfigurator*  configurators[HWC_COMPOSITOR_NUM];

    // the actual compositors
    IConfigurator compNull;
    VideoOverlayConfigurator compVideo;
    SingleLayerConfigurator compSL;
    GenOverlayConfigurator compGen;

    // post-processor, used to adapt buffers to a format suitable to DCC
    sp<IHwcVideoPostprocessor> pp;

    // for fps computation
    size_t  updateCount;
    size_t  updateStart;
    nsecs_t updateNs[UPDATE_STAT_NUM];
    nsecs_t lastDumpNs;
    size_t  lastDumpUpdateCount;

    // to gather basic statistic usage: element 'i' counts
    // how many configurations with i layers were requested
    size_t  config_counts[HWC_STAT_CONF_NUM];

    Updater() :
        configure_count(0),
        disable_hw_composition(false),
        dump_buffers(false),
        dump_fbt(false),
        compNull("null"),
        updateCount(0),
        updateStart(0)
    {
        for (size_t i = 0; i < CONFIG_COUNTERS_NUM ; i++)
            config_counts[i] = 0;
        for (size_t i = 0; i < HWC_COMPOSITOR_NUM ; i++)
            configurators[i] = &compNull;

        configurators[0] = &compSL;
        configurators[1] = &compVideo;
//        configurators[2] = &compGen;
    }

    /**
     * Configure according to the given HWC display contents.
     *
     * At the moment only one layer is supported:
     * - either a single full-screen layer configuration (not counting the FBT)
     * - or the FBT
     */
    void configure(hwc_display_contents_1_t* display) {
        // statistics
        if (display->numHwLayers < CONFIG_COUNTERS_NUM)
            config_counts[display->numHwLayers] += 1;

        // set default: no overlays
        conf.reset(display);

        if (!disable_hw_composition) {
            // try supported use cases in priority order.
            for (size_t i = 0; i < HWC_COMPOSITOR_NUM ; i++) {
                conf.reset(display);
                if (configurators[i]->configure(display, conf)) {
                    conf.configuratorName = configurators[i]->name;
                    break;
                }
            }
        }

        // logging
        configure_count++;
        ALOGV("cnf@%5d (lc=%d oc=%d overlays fbt=%d)", configure_count,
                display->numHwLayers, conf.overlay_count, conf.fbt_enabled);
        for (size_t i = 0; i < conf.overlay_count; i++) {
            ALOGV("  overlay[%d]: index=%d", i, conf.overlayIndexes[i]);
            hwc_dump_layer(&display->hwLayers[conf.overlayIndexes[i]], i);
        }
    }

    void apply(hwc_display_contents_1_t* display) {
        size_t i;
        // mark as overlay the layers that can be handled by DCC
        // (discovered in the configure step), but not the FBT
        for (i = 0; i < conf.overlay_count; i++) {
            if (display->hwLayers[conf.overlayIndexes[i]].compositionType != HWC_FRAMEBUFFER_TARGET)
                display->hwLayers[conf.overlayIndexes[i]].compositionType = HWC_OVERLAY;
        }
        // mark as overlay also the back, if it is not the FBT
        if (conf.back != 0 &&
                display->hwLayers[conf.backIndex].compositionType != HWC_FRAMEBUFFER_TARGET)
            display->hwLayers[conf.backIndex].compositionType = HWC_OVERLAY;
        // and mark as FrameBuffer the rest (except for the FBT, of course)
        for (i = 0; i < display->numHwLayers; i++) {
            if (display->hwLayers[i].compositionType != HWC_FRAMEBUFFER_TARGET
                    && display->hwLayers[i].compositionType != HWC_OVERLAY) {
                display->hwLayers[i].compositionType = HWC_FRAMEBUFFER;

                if (conf.fb_count < HWC_MAX_LAYER_NUM) {
                    conf.fbLayers[conf.fb_count] = &display->hwLayers[i];
                    conf.fb_count++;
                }
            }
        }
    }

    /*
     * update(): called in every circle of prepare to update layer
     * non-geometry changes, like layer handle, crops info, etc.
     */
    void update(hwc_display_contents_1_t* display) {

        // setup layer post-processing if needed
        if (conf.ppLayerIndex != HWC_OVERLAY_INVALID) {
            if (activatePP()) {
                feedPP(conf);
            } else {
                ALOGE("Can not activate PP4HWC: fall back to OpenGL (configurator was %s)", conf.configuratorName);
                conf.reset(display);
            }
        } else {
            deactivatePP();
        }
    }

    void setupSmartComposition(hwc_display_contents_1_t* display) {
         int compositionType = HWC_OVERLAY;

         // If all layers to be HWC_FRAMEBUFFER, bypass smart composition
         if (conf.fb_count >= (display->numHwLayers - 1)) {
             ALOGV("fb layer count=%d, layer list count=%d!\n",
                                         conf.fb_count, display->numHwLayers);
             return;
         }

         // setup smart composition only there's no update on all FB layers
         for (size_t i = 0; i < conf.fb_count; i++) {
             if (!conf.fbLayers[i]) {
                 ALOGE("invalid fb layer[%d]!\n", i);
                 return;
             }

             buffer_handle_t lastHandle = conf.fbLastHandle[i];
             buffer_handle_t currentHandle = conf.fbLayers[i]->handle;

             if (lastHandle != currentHandle) {
                 conf.fbLastHandle[i] = currentHandle;
                 compositionType = HWC_FRAMEBUFFER;
             }
         }

         ALOGV("smart composition enabled %s",
                (compositionType == HWC_OVERLAY) ? "TRUE" : "FALSE");

         // reset compositionType per SmartComposition
         for (size_t i = 0; i < conf.fb_count; i++) {
             conf.fbLayers[i]->compositionType = compositionType;
         }
    }

    /**
     * Render the current configuration to the display using DCC.
     */
    void render(hwc_display_contents_1_t* display) {
        // statistics
        updateNs[updateCount % UPDATE_STAT_NUM] = systemTime();
        updateCount++;
        if (updateCount > UPDATE_STAT_NUM) {
            updateStart++;
        }

        // check if there is anything to render
        if (conf.back == 0 && conf.overlay_count == 0) {
            ALOGE("fbt disabled and no overlay to render");
            return;
        }

        ALOGV("rnd@%5d", configure_count);

        hwc_layer_1_t* back = conf.back;
        if (back == 0) {
            // we still need to provide the FBT for DCC to
            // let it know about the update rect size
            // but we will tell it must be disabled
            back = &display->hwLayers[conf.fbt_index];
        }

        // post update to HW
        hwc_layer_1_t* overlays[HWC_OVERLAY_NUM];
        for (size_t i = 0; i < conf.overlay_count; i++) {
            if (conf.overrides[i] != 0) {
                overlays[i] = conf.overrides[i];
                ALOGV("render() - using override [%p] at %d", overlays[i], i);
            } else {
                overlays[i] = conf.overlays[i];
                ALOGV("render() - using normal   [%p] at %d", overlays[i], i);
            }
        }
        dcc_hal_compose(back, overlays, conf.overlay_count,
                conf.back != 0, &display->retireFenceFd);

        if (pp.get() != 0)
            pp->currentRendered();

        // close fences for all layers we directly render, *after* calling
        // the dcc compose, lest we free the fences before dcc uses them.
        for (size_t i = 0; i < display->numHwLayers; i++) {
            hwc_layer_1_t* layer = &display->hwLayers[i];
            if (layer->compositionType == HWC_OVERLAY &&
                    layer->acquireFenceFd >= 0) {
                close(layer->acquireFenceFd);
                layer->acquireFenceFd = -1;
            }
        }

        for (size_t i = 0; i < conf.overlay_count; i++) {
            hwc_layer_1_t * layer = overlays[i];
            if (layer->acquireFenceFd >= 0) {
                close(layer->acquireFenceFd);
                layer->acquireFenceFd = -1;
            }
        }

        if (conf.back && conf.back->acquireFenceFd >= 0) {
            close(back->acquireFenceFd);
            back->acquireFenceFd = -1;
        }

        // log the release fence fd
        ALOGV("back   rel_fence=%d", (conf.back ? conf.back->releaseFenceFd : -1));
        for (size_t i = 0 ; i < conf.overlay_count; i++) {
            ALOGV("ov[%d] rel_fence=%d", i,
                    (conf.overlays[i] ? conf.overlays[i]->releaseFenceFd : -1));
        }

        // dump layer content if requested
        if (dump_buffers || dump_fbt) {
            for (size_t i = 0; i < display->numHwLayers; i++) {
                hwc_layer_1_t* layer = &display->hwLayers[i];
                String8 filename;
                filename.appendFormat(BUF_DUMP_FILE_PATTERN, i);
                // ignore the composition target buffer
                if (layer->compositionType != HWC_FRAMEBUFFER_TARGET && dump_buffers) {
                    hwc_dump_buffer(layer, filename.string(), i);
                    ALOGD("HWC dumped layer %d to %s", i, filename.string());
                }
                if (layer->compositionType == HWC_FRAMEBUFFER_TARGET && dump_fbt) {
                    hwc_dump_buffer(layer, filename.string(), i);
                    ALOGD("HWC dumped fbt (%d) to %s", i, filename.string());
                }
                for(size_t j = 0; j < conf.overlay_count; j++) {
                    if (conf.overlayIndexes[j] == i && conf.overrides[j] != 0) {
                        // dump alternate layer
                        filename.clear();
                        filename.appendFormat(BUF_DUMP_FILE_PATTERN_OVR, i);
                        hwc_dump_buffer(conf.overrides[i], filename.string(), i);
                    }
                }
            }
            dump_buffers = false;
            dump_fbt = false;
        }
    }

    bool activatePP() {
        if (pp.get() == 0)
            pp = IHwcVideoPostprocessor::create(IHwcVideoPostprocessor::IHWC_VIDEO_PP_TRANSFORMING);
        return pp.get() != 0;
    }

    void deactivatePP() {
        if (pp.get() != 0) {
            // note: the vpp will destroy itself asynchronously
            pp->stopLooping();
            pp.clear();
        }
    }

    void feedPP(LayerConfiguration& conf) {
        const private_handle_t* ppOutBuf;
        // send the video buffer to the post-processor
        // this will also set the release fence of the layer
        pp->submit(conf.overlays[0]);
        // get the current post-processed output
        hwc_layer_1_t*  vl = pp->current(conf.overlays[0]);
        if (vl != 0 && vl->handle != conf.overlays[0]->handle) {
            // the PP has returned a different buffer, tell the
            // configuration about it.
            // the new PP layer looks the same, except for the
            // buffer out, source dimensions and transform
            // TODO: due to the async nature of the VPP the current frame
            // might not match the layer config anymore
            conf.overrides[0] = vl;
            // close the original acquire fence (the VPP holds a duplicate)
            if (conf.overlays[0]->acquireFenceFd >= 0) {
                close(conf.overlays[0]->acquireFenceFd);
                conf.overlays[0]->acquireFenceFd = -1;
            }
        }
    }

    void getFrameRates(float& fpsLastSec, float& fpsTotal, size_t& totalMs) {
        size_t count = updateCount - updateStart;
        if (count < 2) {
            // need at least two samples to compute fps
            fpsLastSec = 0.0;
            fpsTotal = 0.0;
            totalMs = 0;
            return;
        }

        size_t firstIndex = updateStart % UPDATE_STAT_NUM;
        size_t lastIndex = (updateCount - 1) % UPDATE_STAT_NUM;
        nsecs_t firstUpdateNs = updateNs[firstIndex];
        nsecs_t lastUpdateNs  = updateNs[lastIndex];

        // fps from all samples
        totalMs = (size_t) ns2ms(lastUpdateNs - firstUpdateNs);
        fpsTotal = (float) (count - 1) * 1000 / (float) totalMs;
        size_t oneSecIndex;
        for (oneSecIndex = updateCount - 2; oneSecIndex >= updateStart ; oneSecIndex--) {
            if (lastUpdateNs - updateNs[oneSecIndex % UPDATE_STAT_NUM] > s2ns(1))
                break;
        }
        // the index is now pointing to the first item that does
        // not fit into the 1 second range, increase to get the right one
        oneSecIndex++;

        size_t oneSecCount = updateCount - oneSecIndex;
        nsecs_t oneSecUpdateNs = updateNs[oneSecIndex % UPDATE_STAT_NUM];
        size_t oneSecMs = ns2ms(lastUpdateNs - oneSecUpdateNs);
        fpsLastSec = (float) (oneSecCount - 1) * 1000 / (float)(oneSecMs);
    }
};


struct hwc_context_t {
    hwc_context_t() :
        procs(0),
        gralloc_module(0),
        alloc_device(0),
        dcc_fd(-1),
        width(0),
        height(0),
        xdpi(0),
        ydpi(0),
        vsync_period(0)
    {
    }

    hwc_composer_device_1_t device;

	/* hwc_procs_t */
	const hwc_procs_t *procs;
	/* Gralloc */
	const private_module_t *gralloc_module;
	alloc_device_t *alloc_device;

	/* DCC device fd */
	int dcc_fd;

	/* PRIMARY_DISP attributes */
	int width;
	int height;
	int xdpi;
	int ydpi;
	int vsync_period;

	/* VSYNC */
	sp<IntelVsyncThread> mVsyncThread;

	Updater dcc_updater;
    /* stats */
    // the following array counts how many times an
    // update with a given number of layers has happened.
    int update_counts[10];
};


/*****************************************************************************/

static void dump_layer(hwc_layer_1_t const* l) {
    ALOGV("\ttype=%d, flags=%08x, handle=%p, tr=%02x, blend=%04x, {%d,%d,%d,%d}, {%d,%d,%d,%d}",
            l->compositionType, l->flags, l->handle, l->transform, l->blending,
            l->sourceCrop.left,
            l->sourceCrop.top,
            l->sourceCrop.right,
            l->sourceCrop.bottom,
            l->displayFrame.left,
            l->displayFrame.top,
            l->displayFrame.right,
            l->displayFrame.bottom);

	if(l->handle)
	{
		private_handle_t const* hnd = reinterpret_cast<private_handle_t const*>(l->handle);

		ALOGV("HANDLE_DETAILS:: hnd = %p | hnd->ion_fd = %d | hnd->ion_hdl = %0x | hnd->base = %0x | hnd->phys = %0x | hnd->fmt = %d | hnd->[w X h] = [%d X %d] | hnd->bpp = %d \n",
			hnd,
			hnd->ion_fd,
			hnd->ion_hdl,
			hnd->base,
			hnd->phys,
			hnd->fmt,
			hnd->w,
			hnd->h,
			hnd->bpp);
	}


}

static int hwc_prepare(hwc_composer_device_1_t *dev,
			size_t numDisplays,
			hwc_display_contents_1_t** displays) {
    struct hwc_context_t *ctxt = (struct hwc_context_t *) dev;

    if (numDisplays == 0 || displays == 0 || displays[0] == 0)
        return 0;

    // check if geometry is changed, then configure again
    if (displays[0]->flags & HWC_GEOMETRY_CHANGED) {
        ctxt->dcc_updater.configure(displays[0]);
        ctxt->dcc_updater.apply(displays[0]);
    }

    ctxt->dcc_updater.update(displays[0]);
    ctxt->dcc_updater.setupSmartComposition(displays[0]);

    return 0;
}

static int hwc_blank(hwc_composer_device_1_t *dev, int disp, int blank)
{
    (void) dev;
    (void) disp;
    (void) blank;
    return 0;
}

static int hwc_event_control(struct hwc_composer_device_1* dev, int disp,
            int event, int enabled)
{

#if 0
        android::CallStack stack;
        stack.update();
        stack.log(LOG_TAG);
#endif
	ALOGV("%s :: disp:%d | Event:%d | status:%d\n", __func__, disp, event, enabled);
	struct hwc_context_t *pdev = (struct hwc_context_t *)dev;
	switch (event) {
	case HWC_EVENT_VSYNC:
		if (pdev->mVsyncThread != NULL)
			pdev->mVsyncThread->setEnabled((bool)enabled);
		break;
	default:
		break;
	}

	return 0;
}

static void hwc_register_procs(struct hwc_composer_device_1* dev,
				hwc_procs_t const* procs)
{
	struct hwc_context_t *pdev = (struct hwc_context_t *)dev;
	ALOGV("%s\n", __func__);
	pdev->procs = procs;
}

static void hwc_dump_overlay(String8& s, const char* name, const hwc_layer_1_t* l) {
    if (l == 0 || l->handle == 0) {
        s.appendFormat("%s | off | %08x | %08x | %08x |\n", name, 0, 0, 0);
    } else {
        private_handle_t const * h =
                reinterpret_cast<private_handle_t const*>(l->handle);
        s.appendFormat("%s | on  | %08x | %08x | %08x |\n", name, (unsigned int) h, h->phys, h->fmt);
    }
}
/*
Dump Information specific to this HW composer.
*/
static void hwc_dump(struct hwc_composer_device_1* dev, char *buff, int buff_len) {
    struct hwc_context_t *pdev = (struct hwc_context_t *)dev;
    String8 s("Intel HWC:\n");
    LayerConfiguration& conf = pdev->dcc_updater.conf;

    // print statistics on encountered configuration types
    s.append("  ---- update statistics ----\n");
    s.append("  layers: ");
    for (int i = 0 ; i < Updater::HWC_STAT_CONF_NUM ; i++)
        s.appendFormat("%8d |", i);
    s.append("\n  updates:");
    for (int i = 0 ; i < Updater::HWC_STAT_CONF_NUM ; i++)
        s.appendFormat("%8d |", pdev->dcc_updater.config_counts[i]);
    s.append("\n");

    // print statistics on frame rate
    float fpsLastSec;
    float fpsTotal;
    float fpsSinceDump;
    size_t totalMs;
    pdev->dcc_updater.getFrameRates(fpsLastSec, fpsTotal, totalMs);
    nsecs_t t = systemTime();
    fpsSinceDump = (float)(pdev->dcc_updater.updateCount - pdev->dcc_updater.lastDumpUpdateCount) * 1e9 /(float)(t - pdev->dcc_updater.lastDumpNs);
    pdev->dcc_updater.lastDumpNs = t;
    pdev->dcc_updater.lastDumpUpdateCount = pdev->dcc_updater.updateCount;

    s.appendFormat("  Total updates: %d\n  fps (last second): %2.2f\n  fps (last %dms): %2.2f\n  fps (last dump): %2.2f\n",
            pdev->dcc_updater.updateCount,
            fpsLastSec,
            totalMs,
            fpsTotal,
            fpsSinceDump);

    s.append("  ---- current configuration ----\n");

    // print current configuration
    s.appendFormat("  Configurator: %s\n", pdev->dcc_updater.conf.configuratorName);
    s.append("  ov_id | act | handle   | phys     | format   |\n");
    s.append("  ------+-----+----------+----------+----------|\n");
    hwc_dump_overlay(s, "  back ", conf.back);
    hwc_dump_overlay(s, "  ov[0]", conf.getAt(0));
    hwc_dump_overlay(s, "  ov[1]", conf.getAt(1));

    // trigger buffer dump at next display update
    char dumpFlag[PROPERTY_VALUE_MAX];
    property_get("intel.hwc.dumpcontent", dumpFlag, "0");
    if (dumpFlag[0] != '0') {
    	pdev->dcc_updater.dump_buffers = true;
    	s.append("  Enabling buffer dump on next display update\n");
//    	ALOGD("Enabling buffer dump on next display update");
    }
    property_get("intel.hwc.dumpfbt", dumpFlag, "0");
    if (dumpFlag[0] != '0') {
        pdev->dcc_updater.dump_fbt = true;
        s.append("  Enabling FBT dump on next display update\n");
    }
    snprintf(buff, buff_len, "%s", s.string());
}

static int to_bpp_hal_fmt(int hal_pix_fmt) {
    switch (hal_pix_fmt) {
    case HAL_PIXEL_FORMAT_RGBA_8888:
    case HAL_PIXEL_FORMAT_RGBX_8888:
    case HAL_PIXEL_FORMAT_BGRA_8888:
        return 4;
    case HAL_PIXEL_FORMAT_RGB_565:
        return 2;
        /* TBD Complete mapping */
    default:
        ALOGE("%s: Unknown DCC pixel fmt!!\n", __FUNCTION__);
        return -1;
    }
}


static int hwc_set(hwc_composer_device_1_t *dev,
        size_t numDisplays, hwc_display_contents_1_t** displays) {
	struct hwc_context_t *pdev = (struct hwc_context_t *)dev;
    if (numDisplays > 0 || displays && displays[0] && displays[0]->numHwLayers) {
        pdev->dcc_updater.render(displays[0]);
    }
    return 0;
}


static int hwc_get_disp_configs(struct hwc_composer_device_1* dev, int disp,
            uint32_t* configs, size_t* numConfigs) {

	struct hwc_context_t *pdev = (struct hwc_context_t *)dev;

	if(0 == *numConfigs)
		return 0;

	if(HWC_DISPLAY_PRIMARY == disp) {
		configs[0] = 1;
		*numConfigs = 1;
	} else {
		/* No External display for now */
		return -EINVAL;
	}

	return 0;

}


static int hwc_get_disp_attr(struct hwc_composer_device_1* dev, int disp,
            uint32_t config, const uint32_t* attributes, int32_t* values) {

	struct hwc_context_t *pdev = (struct hwc_context_t *)dev;
	int i = 0;

	if((!attributes) ||
		(!values) ||
		(HWC_DISPLAY_PRIMARY != disp) ||
		(1 != config))
		return -1;

	while(HWC_DISPLAY_NO_ATTRIBUTE != attributes[i]) {

		switch(attributes[i]) {

			case HWC_DISPLAY_VSYNC_PERIOD:
				values[i] = pdev->vsync_period;
				break;

			case HWC_DISPLAY_WIDTH:
				values[i] = pdev->width;
				break;

			case HWC_DISPLAY_HEIGHT:
				values[i] = pdev->height;
				break;

			case HWC_DISPLAY_DPI_X:
				values[i] = pdev->xdpi;
				break;

			case HWC_DISPLAY_DPI_Y:
				values[i] = pdev->ydpi;
				break;

			default:
				break;
		}

		i++;
	}


	for(i = 0; HWC_DISPLAY_NO_ATTRIBUTE != attributes[i]; i++) {
		ALOGV("CUST_HWC: values[%d] =  %d\n", i, values[i]);
	}


	return 0;

}




static int dcc_getdispinfo(struct hwc_context_t *pdev)
{

	int ret = -EINVAL;
	struct dcc_hal_attr_t attr;

	/* TODO: For now primary display only */
	ret = dcc_hal_get_attr(DCC_HAL_ATTR_DISPLAY_INFO, &attr);
	if(ret < 0) {
		ALOGE("DCC_IOR_DISPLAY_INFO failed !\n");
		return ret;
	} else {
		ALOGV("Display found wXh: %dx%d xdpi: %d ydpi: %d refresh_rate: %d\n",
 		attr.display_info.width, attr.display_info.height,
		attr.display_info.xdpi, attr.display_info.ydpi,
		attr.display_info.refresh_rate);
		ret = 0;

	}
	/* Populate primary display attributes */
	pdev->width = attr.display_info.width;
	pdev->height = attr.display_info.height;
	pdev->xdpi = attr.display_info.xdpi * 1000; /* pixel per thousand inches */
	pdev->ydpi = attr.display_info.ydpi * 1000; /* pixel per thousand inches */
	pdev->vsync_period = (1000000000 / attr.display_info.refresh_rate); /* in ns */

	return 0;


}

static int dcc_reserve_fb_buffers(struct hwc_context_t *pdev)
{

	struct hw_module_t *pmodule = NULL;
	alloc_device_t *p_alloc_dev = NULL;

	int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, (const struct hw_module_t**)&pmodule);
	ALOGE_IF(err, "[%s] FATAL: can't find the %s module\n", __func__, GRALLOC_HARDWARE_MODULE_ID);
	if (err)
		return -EINVAL;
	/* open alloc device for ion access */
	err = gralloc_open(pmodule, &p_alloc_dev);
	ALOGE_IF(err, "[%s] FATAL: can't open alloc device\n", __func__);
	if (err)
		return -EINVAL;

	/*
	 * Intention: All allocation & buffer mgmt specific things
	 * has to be in gralloc.
	 * FB specific details will stay in hwcomposer.
	 * exceptions:
	 *  1. For now NO FB ioctls are used in hwcomposer.
	 *  2. NUM_FB_BUFFERS should have been defined/generated from here.
	 *	However seems libMali.so tries to get this info from gralloc.
	 */
	private_module_t *m = reinterpret_cast<private_module_t *>(pmodule);
	if (m->reserve_fb) {
		m->reserve_fb(p_alloc_dev, pdev->width, pdev->height, HAL_PIXEL_FORMAT_RGBA_8888);
	} else {
		ALOGE("[%s] Error!! no fb_list init\n", __func__);
		return -EINVAL;
	}

	ALOGV("[%s] FB buffers Reserved\n", __func__);

	return 0;

}

/* Vsync Thread */
hwcVsyncThread::hwcVsyncThread(struct hwc_context_t *pdev) :
		IntelVsyncThread("Vsync thread", pdev)
{ }

hwcVsyncThread::~hwcVsyncThread()
{
	if (vsync_fd >= 0)
		close(vsync_fd);
}

void hwcVsyncThread::setEnabled(bool enabled)
{
	struct dcc_hal_attr_t attr;
	Mutex::Autolock _l(mLock);
	if (mEnabled != enabled) {
		if (true == enabled) {
			attr.vsync = VSYNC_ON;
			//dcc_hal_set_attr(DCC_HAL_ATTR_VSYNC, &attr);
		} else {
			attr.vsync = VSYNC_OFF;
			//dcc_hal_set_attr(DCC_HAL_ATTR_VSYNC, &attr);
		}
		mEnabled = enabled;
		ALOGV("[%s] vsync_en: %d\n", __func__, enabled);
		mCondition.signal();
	}
}

void hwcVsyncThread::onFirstRef()
{
	int err = -1;
	char buf[1024] = {0};
	vsync_fd = open("/sys/bus/platform/drivers/dcc/e1000000.dcc/vsyncts0", O_RDONLY);
	if (vsync_fd < 0) {
		ALOGE("[%s] failed to open vsyncts0 attr\n", __func__);
		return;
	}

	/* Read once */
	err = read(vsync_fd, buf, sizeof(buf));
	if (err < 0) {
		ALOGE("[%s] error in reading vsyncts0 attr: %s\n", __func__, strerror(errno));
		return;
	}

	poll_fds[0].fd = vsync_fd;
	poll_fds[0].events = POLLPRI;

	ALOGV("[%s] VSYNC Thread starts...\n", __func__);
	run("hwcVsyncThread", HAL_PRIORITY_URGENT_DISPLAY);
}

bool hwcVsyncThread::threadLoop()
{
	int err = -1;
	char buf[1024] = {0};
	uint64_t ts = 0;

 	{ // scope for lock
		Mutex::Autolock _l(mLock);
		while (!mEnabled) {
			mCondition.wait(mLock);
		}
	}

	err = poll(poll_fds, 1, -1);
	if ((err > 0) && (poll_fds[0].revents & POLLPRI)) {
		err = lseek(vsync_fd, 0, SEEK_SET);
		if (err < 0) {
			ALOGE("[%s] error in lseek: %s\n", __func__, strerror(errno));
			return false;
		}
		err = read(vsync_fd, buf, sizeof(buf));
		if (err < 0) {
			ALOGE("[%s] error in reading vsyncts0 attr: %s\n", __func__, strerror(errno));
			return false;
		}
		buf[sizeof(buf) - 1] = '\0';
		errno = 0;
		ts = strtoull(buf, NULL, 0);
		if (!errno) {
			if (p_hwc_context->procs)
				p_hwc_context->procs->vsync(p_hwc_context->procs, 0, ts);
				ALOGV("[%s] vsyncts0: %llu\n", __func__, ts);
			} else
				ALOGE("[%s] strtoull returns error\n", __func__);
	} else if (err == -1) {
		if (errno == EINTR)
			return false;
		ALOGE("[%s] error in vsync polling: %s\n", __func__, strerror(errno));
	}

	return true;
}

/* Fake Vsync Thread */
hwcFakeVsyncThread::hwcFakeVsyncThread(struct hwc_context_t *pdev) :
    IntelVsyncThread("Fake Vsync thread", pdev),
    mNextFakeVsync(0),
    mRefreshPeriod(pdev->vsync_period)
{
}

void hwcFakeVsyncThread::setEnabled(bool enabled)
{
    Mutex::Autolock _l(mLock);
    if (mEnabled != enabled) {
        mEnabled = enabled;
        mCondition.signal();
    }
}

void hwcFakeVsyncThread::onFirstRef()
{
	ALOGV("[%s]Fake VSYNC Thread starts...\n", __func__);
	run("hwcFakeVsyncThread", HAL_PRIORITY_URGENT_DISPLAY);
}

bool hwcFakeVsyncThread::threadLoop()
{

 	{ // scope for lock
		Mutex::Autolock _l(mLock);
		while (!mEnabled) {
			mCondition.wait(mLock);
		}
	}

	int err = -1;
	const nsecs_t period = mRefreshPeriod;
	const nsecs_t now = systemTime(CLOCK_MONOTONIC);
	nsecs_t next_vsync = mNextFakeVsync;
	nsecs_t sleep = next_vsync - now;
	if (0 > sleep) {
		// find next vsync
		sleep = (period - ((now - next_vsync) % period));
		next_vsync = now + sleep;
	}
	mNextFakeVsync = next_vsync + period;

	struct timespec spec;
	spec.tv_sec = next_vsync / 1000000000;
	spec.tv_nsec = next_vsync % 1000000000;

	do {
		err = clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &spec, NULL);
	} while ((0 > err) && (EINTR == errno));

	if (0 == err) {
		if (p_hwc_context->procs)
			p_hwc_context->procs->vsync(p_hwc_context->procs, 0, next_vsync);
	}
	return true;
}

static int hwc_device_close(struct hw_device_t *dev) {

	struct hwc_context_t* ctx = (struct hwc_context_t*)dev;

	/* TBD free other resources! */

	if (ctx) {
		if (ctx->mVsyncThread != NULL) {
			ALOGD("[%s] Exit %s\n", __func__, ctx->mVsyncThread->name);
			ctx->mVsyncThread->requestExitAndWait();
			ctx->mVsyncThread.clear();
		}
		free(ctx);
	}
	return 0;
}

/*****************************************************************************/

extern "C" int hwc_device_open(const struct hw_module_t* module, const char* name,
        struct hw_device_t** device) {

    int status = -EINVAL;
    if (!strcmp(name, HWC_HARDWARE_COMPOSER)) {
        struct hwc_context_t *dev = new hwc_context_t();
        if (!dev) {
            ALOGE( "libhwc HAL context creation failed\n");
            return status;
        }

        /* initialize the procs */
        dev->device.common.tag = HARDWARE_DEVICE_TAG;
        dev->device.common.version = HWC_DEVICE_API_VERSION_1_1;
        dev->device.common.module = const_cast<hw_module_t*>(module);
        dev->device.common.close = hwc_device_close;

        dev->device.prepare = hwc_prepare;
        dev->device.set = hwc_set;
        dev->device.blank = hwc_blank;
        dev->device.eventControl = hwc_event_control;
        dev->device.registerProcs = hwc_register_procs;
        dev->device.dump = hwc_dump;

        dev->device.getDisplayConfigs = hwc_get_disp_configs;
        dev->device.getDisplayAttributes = hwc_get_disp_attr;

        *device = &dev->device.common;

        /* DCC device init */
        dev->dcc_fd = dcc_hal_init();
        if (dev->dcc_fd < 0) {
            ALOGE("Can't open device file: %s", DCC_DEVICE_NAME);
            return status;
        }

        /* Get the registered displays */
        status = dcc_getdispinfo(dev);

        /* Reserve the FB buffers */
        status = dcc_reserve_fb_buffers(dev);

        /* Vsync Thread (HW or Fake) */
#if defined(HWC_VSYNC_THREAD)
        dev->mVsyncThread = new hwcVsyncThread(dev);
#elif defined(HWC_FAKE_VSYNC_THREAD)
        dev->mVsyncThread = new hwcFakeVsyncThread(dev);
#else
#error "either HWC_VSYNC_THREAD or HWC_FAKE_VSYNC_THREAD must be defined"
#endif

    }

    ALOGV("CUST_HWC: %s status %d\n", __FUNCTION__, status);
    return status;
}

}; // namespace android

