//#define LOG_NDEBUG 0

#include <hardware/gralloc.h>
#include "gralloc_priv.h"
#include <hardware/hwcomposer.h>
#include <hwcpp.h>

#include <utils/Thread.h>
#include <utils/Vector.h>
#include <utils/Condition.h>
#include <utils/Mutex.h>
#include <utils/FileMap.h>
#define ATRACE_TAG (ATRACE_TAG_HAL | ATRACE_TAG_GRAPHICS)
#include <utils/Trace.h>

#include "hwcomposer_video.h"
#include <sync/sync.h>

namespace android {

#define GRALLOC_ALIGN( value, base ) (((value) + ((base) - 1)) & ~((base) - 1))

class HwcVideoBuffer {
public:
    alloc_device_t *allocDev;
    const private_handle_t *src; // source video buffer
    const private_handle_t *tgt; // target buffer (owned by us)
    uint32_t transform;
    int stride;
    int srcAcquireFenceFd;
    int tgtAcquireFenceFd;
    int tgtReleaseFenceFd;
    uint32_t tgtAcquireValue;
    int32_t id; // for tracing purposes

    int displayFrameLeft; // record displayFrame left
    int displayFrameTop; // record displayFrame top
    int reqW; // requested width
    int reqH; // requested height

    HwcVideoBuffer(alloc_device_t *adev, int32_t bufId) :
        allocDev(adev),
        src(0),
        tgt(0),
        transform(0),
        stride(0),
        srcAcquireFenceFd(-1),
        tgtAcquireFenceFd(-1),
        tgtReleaseFenceFd(-1),
        id(bufId)
        {}

    ~HwcVideoBuffer() {
        free();
        closeFence(tgtAcquireFenceFd, "tgtAcquireFenceFd");
        closeFence(tgtReleaseFenceFd, "tgtReleaseFenceFd");
        closeFence(tgtAcquireFenceFd, "tgtAcquireFenceFd");
    }

    void closeFence(int& fd, const char *fence) {
        if (fd >= 0) {
            int err = close(fd);
            ALOGE_IF(err != 0, "VPP buffer destructor - close %s fence fd error=%d", fence, err);
            fd = -1;
        }
    }

    bool allocate(int w, int h, int fmt, int usage) {
        ATRACE_CALL();
        buffer_handle_t bh;
        int err = allocDev->alloc(allocDev,
                w, h, fmt, GRALLOC_USAGE_HW_COMPOSER | usage,
                &bh, &stride);

        if (err < 0) {
            ALOGE("[%s] HWC video buffer allocation failed, err =%d\n", __func__, strerror(-err));
            tgt = 0;
            return false;
        }
        ALOGV("[%s] HWC video buffer allocated\n", __func__);

        tgt = reinterpret_cast<const private_handle_t*>(bh);
        return true;
    }

    void free() {
        if (tgt != 0) {
            allocDev->free(allocDev, tgt);
            tgt = 0;
        }
    }

    void reconfigure() {
        // reconfigure of target buffer is only needed if:
        // - not yet allocated, or
        // - different dimensions, or
        // - protection usage flag is different
        int srcProt = src->usage & GRALLOC_USAGE_PROTECTED;
        int tgtProt = tgt ? tgt->usage & GRALLOC_USAGE_PROTECTED : 0;
        if (tgt == 0 || reqW != tgt->w || reqH != tgt->h || srcProt != tgtProt) {
            free();
            allocate(reqW, reqH, HAL_PIXEL_FORMAT_RGBA_8888, srcProt);
        }
    }

    void waitReady() {
        ATRACE_CALL();
        // make sure the source buffer is ready
        if (srcAcquireFenceFd >= 0) {
            int result = sync_wait(srcAcquireFenceFd, 200);
            ALOGE_IF(result, "PPHWC: timeout waiting for source ready");
            ALOGV("VPP waitBufferReady (src) done: %d", result);
            close(srcAcquireFenceFd);
            srcAcquireFenceFd = -1;
        }
        // make sure the destination is not hold by DCC
        if (tgtReleaseFenceFd >= 0) {
            ALOGV("VPP waitBufferReady");
            int result = sync_wait(tgtReleaseFenceFd, 200);
            ALOGE_IF(result, "PPHWC: timeout waiting for target ready");
            ALOGV("VPP waitBufferReady done: %d, on %d", result, tgtReleaseFenceFd);
            close(tgtReleaseFenceFd);
            tgtReleaseFenceFd = -1;
        }
    }

    uint32_t getSourceStride() {
        return align(src->w, src->fmt);
    }

    uint32_t getSourceSliceHeight() {
        return src->h;
    }

    uint32_t getSourceBpp() {
        uint32_t bpp;
        align(src->w, src->fmt, &bpp);
        return bpp;
    }

    uint32_t getTargetStride() {
        return align(tgt->w, tgt->fmt);
    }

    uint32_t getTargetSliceHeight() {
        return tgt->h;
    }

    uint32_t getTargetBpp() {
        uint32_t bpp;
        align(tgt->w, tgt->fmt, &bpp);
        return bpp;
    }

    static uint32_t align(uint32_t w, uint32_t halFmt, uint32_t* outBpp = 0) {
        uint32_t stride, bpp, bpr, base;
        switch (halFmt) {
        case HAL_PIXEL_FORMAT_RGBA_8888:
        case HAL_PIXEL_FORMAT_RGBX_8888:
        case HAL_PIXEL_FORMAT_BGRA_8888:
            bpp = 4;
            base = 8;
            break;
        case HAL_PIXEL_FORMAT_RGB_888:
            bpp = 3;
            base = 8;
            break;
        case HAL_PIXEL_FORMAT_RGB_565:
        case HAL_PIXEL_FORMAT_RAW16:
            base = 2;
            bpp = 2;
            break;
        case HAL_PIXEL_FORMAT_YV12:
        case HAL_PIXEL_FORMAT_YCrCb_420_SP:
        case HAL_PIXEL_FORMAT_YCbCr_420_SP:
        case HAL_PIXEL_FORMAT_YCbCr_422_SP:
        case HAL_PIXEL_FORMAT_YCbCr_422_I:
            bpp = 1;
            base = 16;
            break;
        default:
            bpp = 1;
            base = 1;
            break;
        }
        bpr = GRALLOC_ALIGN(w * bpp, base);
        stride = bpr / bpp;
        if (outBpp)
            *outBpp = bpp;
        return stride;
    }
};

#if 0
class HwcVideoFifo {
public:
    enum {
        NUM_BUFFERS = 3
    };
    HwcVideoBuffer* buffers[NUM_BUFFERS];
    int count;
    int offset;

    bool queue(HwcVideoBuffer* b) {
        if (count < NUM_BUFFERS) {
            buffers[]
        }
    }
};
#endif

class HwcBaseVideoPostprocessor : public Thread, public IHwcVideoPostprocessor {
public:
    enum {
        NUM_BUFFERS = 3
    };
    alloc_device_t *allocDev;
    Vector<HwcVideoBuffer*> mQueue;
    Vector<HwcVideoBuffer*> mFree;
    HwcVideoBuffer* mCurrent;
    Condition mEvent;
    Condition mFirstBuffer;
    Mutex mMutex;
    bool shouldLoop;
    hwc_layer_1_t mCurrentLayer;
    int queuedCount;
    buffer_handle_t lastSubmitted;
    int lastSubmittedWidth;
    int lastSubmittedHeight;
    uint32_t lastSubmittedTransform;

    HwcBaseVideoPostprocessor() :
        allocDev(0),
        mCurrent(0),
        shouldLoop(true),
        queuedCount(0),
        lastSubmitted(0),
        tmp(0) {
        const struct hw_module_t *pmodule = NULL;

        memset(&mCurrentLayer, 0, sizeof(mCurrentLayer));


        int err = hw_get_module(GRALLOC_HARDWARE_MODULE_ID, &pmodule);
        ALOGE_IF(err, "VPP create: error, can't find %s\n", GRALLOC_HARDWARE_MODULE_ID);
        if (err) {
            shouldLoop = false;
            return;
        }
        err = gralloc_open(pmodule, &allocDev);
        ALOGE_IF(err, "VPP create: error, can't open alloc device\n");
        if (err) {
            shouldLoop = false;
            return;
        }
        // push NUM_BUFFERS empty (and un-allocated) buffers in the free pool
        for (int32_t i = 1 ; i <= NUM_BUFFERS ; i++) {
            HwcVideoBuffer* b = new HwcVideoBuffer(allocDev, i);
            mFree.push(b);
        }


        ALOGD("VPP create: OK, using %d buffers", NUM_BUFFERS);
    }

    virtual ~HwcBaseVideoPostprocessor() {
        ATRACE_CALL();
        HwcVideoBuffer* b;
        b = dequeueFreeBuffer();
        while (b != 0) {
            delete b;
            b = dequeueFreeBuffer();
        }
        b = dequeueQueuedBuffer();
        while (b != 0) {
            delete b;
            b = dequeueQueuedBuffer();
        }
        if(mCurrent != 0) {
            delete mCurrent;
            mCurrent = 0;
        }

        if (allocDev != 0)
            gralloc_close(allocDev);
        ALOGD("VPP delete: OK");

        // free tmp buffer
        freeTempBuffer();
    }

    status_t readyToRun()
    {
        ALOGD("VPP start: OK");
        return NO_ERROR;
    }

    HwcVideoBuffer* dequeueFreeBuffer() {
        HwcVideoBuffer* b = 0;
        if (mFree.size() > 0) {
            b = mFree.itemAt(0);
            mFree.removeAt(0);
        }
        return b;
    }

    HwcVideoBuffer* dequeueQueuedBuffer() {
        HwcVideoBuffer* b = 0;
        if (mQueue.size() > 0) {
            b = mQueue.itemAt(0);
            mQueue.removeAt(0);
        }
        return b;
    }

    HwcVideoBuffer* getLastQueuedBufferLocked() {
        HwcVideoBuffer* b = 0;
        if (mQueue.size() > 0) {
            b = mQueue.itemAt(mQueue.size() - 1);
        }
        return b;
    }

    const private_handle_t* hndl(const hwc_layer_1_t* l) const {
        return reinterpret_cast<const private_handle_t*>(l->handle);
    }

    bool submit_unlocked(const hwc_layer_1_t* l) {
        ATRACE_CALL();
        int w = l->displayFrame.right - l->displayFrame.left;
        int h = l->displayFrame.bottom - l->displayFrame.top;
        // do not submit the same frame twice, unless we have
        // different post-processor parameters
        if (l->handle == lastSubmitted &&
                w == lastSubmittedWidth &&
                h == lastSubmittedHeight &&
                l->transform == lastSubmittedTransform)
            return true;
        ATRACE_INT("VPPFree", mFree.size());
        HwcVideoBuffer* b = dequeueFreeBuffer();
        ALOGV("VPP submit - got free buffer: %p", b);
        if (b != 0) {
            lastSubmitted = l->handle;
            lastSubmittedWidth = w;
            lastSubmittedHeight = h;
            lastSubmittedTransform = l->transform;
            b->src = hndl(l);
            b->reqW = w;
            b->reqH = h;
            b->displayFrameLeft = l->displayFrame.left;
            b->displayFrameTop = l->displayFrame.top;
            // TODO: temporary limitations (hopefully) until DCC can crop
            b->reqW &= ~7; // multiple of 8 for rgb565
            b->reqH &= ~1; // multiple of 2 for rgb565
            b->transform = l->transform;
            mQueue.push_back(b);
            ATRACE_INT("VPP-QFrame", b->id);
            queuedCount++;
            setTargetAcquireFence(b);
            // duplicate the acquire fence from the original layer
            if (l->acquireFenceFd >= 0)
                b->srcAcquireFenceFd = dup(l->acquireFenceFd);
            else
                b->srcAcquireFenceFd = -1;
            ALOGV("VPP submit - pushed, src=%p, tgt=%p, acq=%d@%d", b->src, b->tgt,
                    b->tgtAcquireFenceFd, b->tgtAcquireValue);
            return true;
        } else
            ALOGW("VPP submit: No free buffer, dropping frame");
        return false;
    }

    virtual void setTargetAcquireFence(HwcVideoBuffer* b) {
        // default: do nothing
        (void) b;
    }

    bool submit(const hwc_layer_1_t* l) {
        ATRACE_CALL();
        ALOGV("VPP submit");
        if (!shouldLoop ||
                l == 0 ||
                l->handle == 0 ||
                !isSupportedInput(*hndl(l)))
            return false;

        // allocate temp buffer for scaling if needed
        if (isScaleNeeded(l)) {
            ALOGV("temp buffer for scaling needed");
            int srcProt = hndl(l)->usage & GRALLOC_USAGE_PROTECTED;
            if (tmp == 0) {
                allocTempBuffer(srcProt);
            }
            reconfigureTempbuf(hndl(l)); // check if the usage flag is different from the past
            if (tmp == 0) {
                ALOGE("temp buffer allocation failed. return false. ");
                return false;
            }
        }
        else
            ALOGV("temp buffer for scaling not needed.");

        mMutex.lock();
        if (submit_unlocked(l)) {
            ALOGV("VPP submit - signal");
            mEvent.signal();
        }
        mMutex.unlock();
        return true;
    }

    bool isBufferMatching(const hwc_layer_1_t* origl, HwcVideoBuffer *b) {
        return (origl->transform == b->transform) &&
          ((origl->displayFrame.right - origl->displayFrame.left) == b->tgt->w) &&
          ((origl->displayFrame.bottom - origl->displayFrame.top) == b->tgt->h);
    }

    virtual hwc_layer_1_t* current(const hwc_layer_1_t* origl) {
        hwc_layer_1_t* vl = 0;
        ATRACE_CALL();
        AutoMutex lock(mMutex);

        mCurrentLayer = *origl;
        HwcVideoBuffer *b = mCurrent;
        ALOGV("VPP current() : %p", b);
        if (b == 0) {
            // no buffer available yet, wait for one, if the queue is not empty
            if (queuedCount > 0) {
                ALOGV("VPP current() - wait for first buffer");
                mFirstBuffer.wait(mMutex);
                b = mCurrent;
            } else
                ALOGW("VPP current() - No buffer enqueued");
        }
        // TODO: wait for an output image matching the current size if necessary
//        while (b != 0 && !isBufferMatching(origl, b)) {
//            // the current buffer is no longer matching the display
//        }
        if (b == 0) {
            // should actually never happen
            ALOGE("First buffer signaled, but mCurrent is still null");
        } else if (b->tgt == 0) {
            // could happen in case of out of memory
            ALOGE("No target buffer, skipping video layer");
            vl = 0;
        } else {
            vl = &mCurrentLayer;
            /* give a duplicate of the target acquire fence */
            if (b->tgtAcquireFenceFd >= 0) {
                vl->acquireFenceFd = dup(b->tgtAcquireFenceFd);
                ALOGD("vl->acquireFenceFd = dup(%d) : %d", b->tgtAcquireFenceFd, vl->acquireFenceFd);
            }
            else
                vl->acquireFenceFd = -1;
            vl->handle = b->tgt;
            vl->sourceCrop.top = 0;
            vl->sourceCrop.left = 0;
            vl->sourceCrop.right = b->tgt->w;
            vl->sourceCrop.bottom = b->tgt->h;
            vl->transform = 0;
            vl->displayFrame.left = b->displayFrameLeft;
            vl->displayFrame.top = b->displayFrameTop;
            // TODO: temporary limitations (hopefully) until DCC can crop
            vl->displayFrame.right = vl->displayFrame.left + b->tgt->w;
            vl->displayFrame.bottom = vl->displayFrame.top + b->tgt->h;
            ATRACE_INT("VPP-CFrame", b->id);
            ALOGV("VPP current() - vl.handle=%p" , vl->handle);
        }
        return vl;
    }

    virtual void currentRendered() {
        AutoMutex lock(mMutex);
        int bufferFd = mCurrent->tgtReleaseFenceFd;
        // note: mCurrentLayer has been submitted to DCC, and
        // got its releaseFenceFd initialised by the driver.
        int layerFd = mCurrentLayer.releaseFenceFd;
        ALOGV("currentRendered() : nfd=%d, ofd=%d", layerFd, bufferFd);

        if (mCurrentLayer.handle != mCurrent->tgt) {
            // the current PP buffer has changed in the meantime, close
            // the layer release fence fd, if necessary.
            // TODO: for correctness this should be merged with
            // the corresponding PP buffer to make sure we do not re-use it
            // before DCC is finished
            if (layerFd >= 0) {
                close(layerFd);
                mCurrentLayer.releaseFenceFd = -1;
                layerFd = -1;
            }
        }

        // now update the release fence by merging previous and new fences
        if (layerFd >= 0 && bufferFd >= 0) {
            int mergedFd = sync_merge("hwc-vpp", bufferFd, layerFd);
            ALOGE_IF((mergedFd < 0), "Could not merge release fences!");
            mCurrent->tgtReleaseFenceFd = mergedFd;
            close(layerFd);
            close(bufferFd);
        } else if (layerFd >= 0) {
            // bufferFd is not valid, replace it with layerFd
            mCurrent->tgtReleaseFenceFd = layerFd;
        }
    }

    void stopLooping() {
        ALOGV("stopLooping()");
        shouldLoop = false;
        mEvent.signal();
        requestExit();
    }

    /*
     * Main thread loop:
     * - wait for a signal (usually incoming buffer)
     * - get next buffer in queue
     * - post process buffer
     * - set as current buffer for compositions
     * - trigger composition if possible
     * TODO: would be nice to trigger the SF composition, but
     * probably we have no access to SF from down here...
     */
    virtual bool threadLoop() {
        HwcVideoBuffer* b = 0;
        mMutex.lock();
        b = dequeueQueuedBuffer();
        if (b == 0) {
            // wait for an incoming event/buffer
            ALOGV("threadLoop() - wait");
            mEvent.wait(mMutex);
            b = dequeueQueuedBuffer();
        }
        ALOGV("threadLoop() - got %p", b);
        while (shouldLoop && b != 0) {
            mMutex.unlock();
            // make sure nobody is holding this buffer
            b->waitReady();
            // reconfigure the buffer if necessary
            // note: in case of out of memory the target buffer might be
            // no longer valid!
            b->reconfigure();
            ATRACE_INT("VPP-PFrame", b->id);
            // do the processing
            processBuffer(*b);
            mMutex.lock();
            // now update current
            if (mCurrent != 0)
                mFree.push_back(mCurrent);
            else
                mFirstBuffer.broadcast();
            mCurrent = b;
            queuedCount--;
            // we can safely close the tgt acquire fence fd now
            if (b->tgtAcquireFenceFd >= 0) {
                close(b->tgtAcquireFenceFd);
                b->tgtAcquireFenceFd = -1;
            }

            b = 0;
            if(shouldLoop)
                b = dequeueQueuedBuffer();
            ALOGV("threadLoop() - got %p", b);
        }
        mMutex.unlock();
        return shouldLoop;
    }

    virtual void processBuffer(HwcVideoBuffer& b) = 0;
    virtual bool isSupportedInput(const private_handle_t& h) = 0;

    const private_handle_t* getTempBuf() {
        return tmp;
    }

private:
    static const int DISPLAY_WIDTH=1280;
    static const int DISPLAY_HEIGHT=720;
    /*
    The intermediate buffer must, in the worst case, accommodate an image that is
    downscaling to a size that is smaller than the display size.
    */
    const private_handle_t* tmp; // temp buffer per session for up/down scaling (owned by us)

    int needsUpAndDownScale(int in_w, int in_h, int out_w, int out_h) const {
        return ((in_w > out_w) && (in_h < out_h)) ||
               ((in_w < out_w) && (in_h > out_h));
    }
    uint32_t getMaxUpscaledWidth(uint32_t w) const {
        return w*3;
    }
    uint32_t getMaxUpscaledHeight(uint32_t h) const {
        return (h == 0) ? 0 : (3 * h - 2);
    }
    bool isScaleNeeded(const hwc_layer_1_t* l) const {
        // allocate temp buffer for upscaling if needed
        uint32_t in_width = hndl(l)->w, in_height = hndl(l)->h;
        uint32_t out_width = static_cast<uint32_t>(l->displayFrame.right-l->displayFrame.left);
        uint32_t out_height= static_cast<uint32_t>(l->displayFrame.bottom-l->displayFrame.top);
        if (l->transform == HAL_TRANSFORM_ROT_270 || l->transform == HAL_TRANSFORM_ROT_90) {
            uint32_t temp=out_width;
            out_width = out_height;
            out_height = temp;
        }

        return needsUpAndDownScale(in_width, in_height, out_width, out_height) ||
                out_width  > getMaxUpscaledWidth(in_width) ||  out_height > getMaxUpscaledHeight(in_height);
    }

    bool allocTempBuffer(int usage) {
        // allocate tmp buffer
        buffer_handle_t bh_tmp;
        int stride = 0;
        int err = allocDev->alloc(allocDev,
                DISPLAY_WIDTH, DISPLAY_HEIGHT, HAL_PIXEL_FORMAT_YCbCr_420_SP, GRALLOC_USAGE_HW_COMPOSER | usage,
                &bh_tmp, &stride);

        if (err < 0) {
            ALOGE("[%s] HWC video buffer (tmp) allocation failed, err =%d\n", __func__, strerror(-err));
            tmp = 0;
            return false;
        }
        ALOGV("[%s] HWC video buffer(tmp) allocated: %s\n", __func__, usage==0?"normal(not secure) buffer":"secure buffer");

        tmp = reinterpret_cast<const private_handle_t*>(bh_tmp);
        return true;
    }

    void freeTempBuffer() {
        if (tmp != 0) {
            allocDev->free(allocDev, tmp);
            tmp = 0;
            ALOGV("[%s] HWC video buffer(tmp) freed\n", __func__);
        }
    }

    // in case secure playback switches to normalplayback in the middile, or vice versa
    // which rarely happens in practice
    void reconfigureTempbuf(const private_handle_t* src) {
        // reconfigure of target buffer is only needed if:
        // - protection usage flag is different
        int srcProt = src->usage & GRALLOC_USAGE_PROTECTED;
        int tmpProt = tmp ? tmp->usage & GRALLOC_USAGE_PROTECTED : 0;
        if (srcProt != tmpProt) {
            freeTempBuffer();
            allocTempBuffer(srcProt);
        }
    }
};

class HwcVideoPostprocessor : public HwcBaseVideoPostprocessor {
public:
    struct pphwc_s *pphwcInst;
    HwcVideoPostprocessor() :
        HwcBaseVideoPostprocessor(),
        pphwcInst(0) {
        int err = pphwc_init(&pphwcInst);
        if (err != OK) {
            ALOGE("PPHWC init: error=%d", err);
            shouldLoop = false;
        }
        else
            ALOGI("PPHWC init: OK");
    }
    virtual ~HwcVideoPostprocessor() {
        int res = 0;
        if (pphwcInst != 0) {
            res = pphwc_free(pphwcInst);
        }
        if (res != 0)
            ALOGE("PPHWC free: error=%d (inst=%p)", res, pphwcInst);
        else
            ALOGI("PPHWC free: OK (inst=%p)", pphwcInst);
    }
    virtual bool isSupportedInput(const private_handle_t& h) {
        // VPU HW post-processor only takes YUV, not RGB, as input
        return h.fmt == HAL_PIXEL_FORMAT_YCbCr_420_SP;
    }
    virtual void setTargetAcquireFence(HwcVideoBuffer* b) {
        int res;

        res = pphwc_prepare(pphwcInst,
            &b->tgtAcquireFenceFd,
            &b->tgtAcquireValue);
        if (res < 0) {
            ALOGE("PPHWC prepare: error=%d (inst=%p)", res, pphwcInst);
        }
    }
    virtual void processBuffer(HwcVideoBuffer& b) {
        ATRACE_CALL();
        struct hwcInputPic in;
        struct hwcOutputPic out;

        if (b.tgt == 0) {
            ALOGE("processBuffer: no target buffer");
            return;
        }

        in.busAddress = b.src->phys;
        in.width = b.src->w;
        in.height = b.src->h;
        in.stride = b.getSourceStride();
        in.sliceHeight = b.getSourceSliceHeight();
        in.format = b.src->fmt;

        out.busAddress = b.tgt->phys;
        out.width = b.tgt->w;
        out.height = b.tgt->h;
        out.stride = b.tgt->w;
        out.sliceHeight = b.getTargetSliceHeight();
        out.format = b.tgt->fmt;
        out.rotation = b.transform;

        ALOGV("processBuffer(): in{@%x, (%d,%d) (%d,%d), %x } -%x-> out{@%x, (%d,%d) (%d,%d), %x }",
                in.busAddress, in.width, in.height, in.stride, in.sliceHeight, in.format,
                out.rotation,
                out.busAddress, out.width, out.height, out.stride, out.sliceHeight, out.format);
        ALOGV("processBuffer() - got %p", &b);

        // use temp buffer if it has been allocated
        int res = 0;
        const private_handle_t* tmp = getTempBuf();
        if (tmp!=0)
            int res = pphwc_post_process(pphwcInst, &in, &out, b.tgtAcquireValue, (uint8_t*)tmp->phys);
        else
            int res = pphwc_post_process(pphwcInst, &in, &out, b.tgtAcquireValue, NULL);

        if (res != 0) {
            ALOGE("processBuffer(): in{@%x, (%d,%d) (%d,%d), %x } -%x->  out{@%x, (%d,%d) (%d,%d), %x }",
                    in.busAddress, in.width, in.height, in.stride, in.sliceHeight, in.format,
                    out.rotation,
                    out.busAddress, out.width, out.height, out.stride, out.sliceHeight, out.format);
            ALOGE("processBuffer: error=%d", res);
        } else
            ALOGV("processBuffer() - pphwc returned %d", res);
    }

};

/*
A simplistic post processor that does not scale/rotate.
It simply copies the input (cropping if necessary).
*/
class HwcCopyVideoPostprocessor : public HwcBaseVideoPostprocessor {
public:
    HwcCopyVideoPostprocessor() :
        HwcBaseVideoPostprocessor() {
    }
    virtual bool isSupportedInput(const private_handle_t& h) {
        // copy PP only supports RGB565
        return h.fmt == HAL_PIXEL_FORMAT_RGB_565;
    }
    virtual void processBuffer(HwcVideoBuffer& b) {
        ATRACE_CALL();
        struct hwcInputPic in;
        struct hwcOutputPic out;

        in.busAddress = b.src->phys;
        in.width = b.src->w;
        in.height = b.src->h;
        in.stride = b.getSourceStride();
        in.sliceHeight = b.getSourceSliceHeight();
        in.format = b.src->fmt;

        out.busAddress = b.tgt->phys;
        out.width = b.tgt->w;
        out.height = b.tgt->h;
        out.stride = b.getTargetStride();
        out.format = b.tgt->fmt;
        out.rotation = b.transform;

        uint32_t bpp = b.getTargetBpp();
        int w = bpp * (out.width > in.width ? in.width : out.width);
        int h = out.height > in.height ? in.height : out.height;
        int dstClear = bpp * (out.width - w);
        int dstStride = bpp * b.getTargetStride();
        int srcStride = bpp * b.getSourceStride();
        char *dst = (char *)b.tgt->base;
        char *src = (char *)b.src->base;
        for (int y = 0 ; y < h; y++) {
            memcpy(dst, src, w);
            if (dstClear > 0) {
                memset(dst + w, 0, dstClear);
            }
            src += srcStride;
            dst += dstStride;
        }
    }
};

sp<IHwcVideoPostprocessor> IHwcVideoPostprocessor::create(enum IHwcVideoPPType type) {
    sp<HwcBaseVideoPostprocessor> vpp;
    switch (type) {
        case IHWC_VIDEO_PP_CROP_COPY:
            vpp = new HwcCopyVideoPostprocessor();
            break;
        case IHWC_VIDEO_PP_TRANSFORMING:
            vpp = new HwcVideoPostprocessor();
            break;
        default:
            break;
    }
    if (vpp.get() != 0 && !vpp->shouldLoop) {
        // something went wrong
        vpp.clear();
    } else
        vpp->run("hwc-vpp", HAL_PRIORITY_URGENT_DISPLAY);
    return vpp;
}

IHwcVideoPostprocessor::~IHwcVideoPostprocessor() {
}

}
