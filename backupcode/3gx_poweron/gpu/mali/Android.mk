# Copyright (C) 2014 Intel Mobile Communications GmbH
ifneq 'mali-midgard' '$(findstring mali-midgard, $(TARGET_BOARD_PLATFORM_GPU))'

USE_MALI_GFX :=

ifeq ($(BOARD_USE_MALI_GFX),true)
USE_MALI_GFX := true
endif

#ifeq ($(TARGET_DEVICE),Sf3gr_sr_garnet)
ifneq ($(filter Sf3gr_sr_garnet Sf3gr_svb_n1 Sf3gr_mrd_n1, $(TARGET_DEVICE)),)
USE_MALI_GFX := true
endif

ifeq ($(USE_MALI_GFX),true)

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
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT)/lib/egl
include $(BUILD_PREBUILT)

endif
endif # BOARD_USE_MALI_GFX
endif
