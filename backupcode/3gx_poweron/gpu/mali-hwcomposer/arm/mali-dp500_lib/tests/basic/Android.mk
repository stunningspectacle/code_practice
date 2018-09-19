LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES := main.c \
		graphics.c \
		graphics_adf.c

#		graphics_fbdev.c \
#		events.c \
#		resources.c

#LOCAL_C_INCLUDES +=
#    external/libpng\
#    external/zlib

LOCAL_STATIC_LIBRARIES := libadf
LOCAL_SHARED_LIBRARIES := libion

LOCAL_MODULE := displaytest

# This used to compare against values in double-quotes (which are just
# ordinary characters in this context).  Strip double-quotes from the
# value so that either will work.

#
#	DP500 doesnt support RGBX or BGRA
#	gr_fill filling pattern is for BGR
#	so choosing XBGR
#
LOCAL_CFLAGS += -DRECOVERY_XBGR

#ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),RGBX_8888)
#LOCAL_CFLAGS += -DRECOVERY_RGBX
#endif
#ifeq ($(subst ",,$(TARGET_RECOVERY_PIXEL_FORMAT)),BGRA_8888)
#  LOCAL_CFLAGS += -DRECOVERY_BGRA
#endif


include $(BUILD_EXECUTABLE)
