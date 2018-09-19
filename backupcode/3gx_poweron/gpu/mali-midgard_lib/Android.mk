# Copyright (C) 2015 Intel Mobile Communications GmbH

LOCAL_PATH := $(call my-dir)

# Only use Mali Midgard, if TARGET_BOARD_PLATFORM_GPU contains mali-midgard
ifeq 'mali-midgard' '$(findstring mali-midgard, $(TARGET_BOARD_PLATFORM_GPU))'

# Only use precompiled libraries if Mali Midgard DDK source is not present
ifeq ($(wildcard  $(LOCAL_PATH)/PRIVATE/mali-midgard_src/driver/product/Android.mk),)

include $(CLEAR_VARS)
LOCAL_MODULE := libGLES_mali
LOCAL_SRC_FILES := lib/$(LOCAL_MODULE).so
LOCAL_MODULE_TAGS := optional eng debug
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/lib/egl
LOCAL_POST_INSTALL_CMD := $(hide) cd $(TARGET_OUT_VENDOR_SHARED_LIBRARIES); \
        ln -sf /vendor/lib/egl/libGLES_mali.so libOpenCL.so.1.1; \
        ln -sf libOpenCL.so.1.1 libOpenCL.so.1; \
        ln -sf libOpenCL.so.1 libOpenCL.so
include $(BUILD_PREBUILT)

##############################################
include $(CLEAR_VARS)
LOCAL_MODULE := mali_clcc 
LOCAL_SRC_FILES := bin/$(LOCAL_MODULE) 
LOCAL_MODULE_TAGS := optional eng debug
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT)/bin
include $(BUILD_PREBUILT)

##############################################
include $(CLEAR_VARS)
LOCAL_MODULE := libRSDriverArm
LOCAL_MODULE_TAGS := optional eng
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/lib
LOCAL_SRC_FILES := lib/$(LOCAL_MODULE).so
include $(BUILD_PREBUILT)

##############################################
include $(CLEAR_VARS)
LOCAL_MODULE := libbccArm
LOCAL_MODULE_TAGS := optional eng
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/lib
LOCAL_SRC_FILES := lib/$(LOCAL_MODULE).so
include $(BUILD_PREBUILT)

#############################################
include $(CLEAR_VARS)
LOCAL_MODULE := libmalicore
LOCAL_MODULE_TAGS := optional eng
LOCAL_MODULE_SUFFIX := .bc
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT)/vendor/lib
LOCAL_SRC_FILES := lib/$(LOCAL_MODULE).bc
include $(BUILD_PREBUILT)

# Include Mali Midgard DDK source makefile
else

include $(LOCAL_PATH)/PRIVATE/mali-midgard_src/driver/product/Android.mk

endif

endif
