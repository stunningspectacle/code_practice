#ifndef HWCOMPOSER_VIDEO_H_
#define HWCOMPOSER_VIDEO_H_


namespace android {

/*
 * This class implements a thread that can postprocess video buffers
 * to match the size and format of the display region where they are
 * supposed to be shown.
 *
 * It allows then the display controller to render them as overlays
 * without the need of scaling/rotating, which the DCC can't do.
 *
 * Buffers should be queued using queueBuffer(), the conversion is then
 * performed asynchronously in the thread and as soon as one is finished,
 * it is stored as the current video buffer.
 *
 * HWC should always use the current buffer when rendering the display,
 * not wait for the actual one to be post-processed, this way the video will
 * likely always be one frame behind.
 */
class IHwcVideoPostprocessor : virtual public RefBase {
public:
    enum IHwcVideoPPType {
        IHWC_VIDEO_PP_CROP_COPY,
        IHWC_VIDEO_PP_TRANSFORMING
    };
    static sp<IHwcVideoPostprocessor> create(enum IHwcVideoPPType type);

    /**
     * Queue the buffer associated by this layer for
     * post-processing.
     */
    virtual bool submit(const hwc_layer_1_t* l) = 0;

    /**
     * Get the frame that is currently ready for display,
     * i.e. it has been post-processed.
     *
     * note: the returned layer is only valid until the next call
     * to current().
     *
     * @param vl the original layer that should display the
     *           current frame
     * @return another layer structure suitable to display
     *         the post-processed current frame
     */
    virtual hwc_layer_1_t* current(const hwc_layer_1_t* vl) = 0;

    /**
     * Notify the post-processor that the current layer has
     * been updated with the fenceFd returned by DCC.
     */
    virtual void currentRendered() = 0;

    virtual void stopLooping() = 0;
    virtual ~IHwcVideoPostprocessor();
};
}

#endif /* HWCOMPOSER_VIDEO_H_ */
