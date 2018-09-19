# Copyright (C) 2014 Intel Mobile Communications GmbH

LOCAL_PATH := $(call my-dir)

# Only use precompiled Mali Utgard libs, if source is not present
ifeq ($(wildcard hardware/arm/mali/driver/Android.mk),)

include $(CLEAR_VARS)
LOCAL_MODULE := libGLES_mali
ifeq ($(TARGET_BUILD_TYPE),debug)
LOCAL_SRC_FILES := lib/debug/$(LOCAL_MODULE).so
else
LOCAL_SRC_FILES := lib/release/$(LOCAL_MODULE).so
endif
LOCAL_MODULE_TAGS := optional eng debug
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/egl
include $(BUILD_PREBUILT)

endif

