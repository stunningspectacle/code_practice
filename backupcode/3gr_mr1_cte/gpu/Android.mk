# Copyright (C) 2015 Intel Mobile Communications GmbH

LOCAL_PATH := $(call my-dir)

# Fallback to old Mali Utgard include, if TARGET_BOARD_PLATFORM_GPU is not defined
ifeq ($(TARGET_BOARD_PLATFORM_GPU),)

$(info "Fallback to old Mali Utgard include")
include $(LOCAL_PATH)/arm/mali/Android.mk

else

# Include Android.mk of precombiled libs of corresponding GPU variant
ifneq ($(wildcard $(LOCAL_PATH)/$(TARGET_BOARD_PLATFORM_GPU)_lib/Android.mk),)

include $(LOCAL_PATH)/$(TARGET_BOARD_PLATFORM_GPU)_lib/Android.mk

else

# TARGET_BOARD_PLATFORM_GPU set, but cannot find the corresponing Android.mk file
# Library repository not in Manifest?
$(error "'$(LOCAL_PATH)/$(TARGET_BOARD_PLATFORM_GPU)_lib/Android.mk' not present!")

endif

endif

