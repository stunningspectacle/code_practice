# Copyright (C) 2013, 2015 Intel Mobile Communications GmbH
#
# Copyright (C) 2008 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.


#ifeq '$(findstring true, $(FEAT_INTEL_HWC))' 'true'

LOCAL_PATH := $(call my-dir)

# HAL module implemenation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libEGL libutils libhardware libsync libcutils libion

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(TOP)/system/core/libsync \
	hardware/intel/vpu/verisilicon/g1-v6_g2-v1_h1-v6_lib/inc/ \
	hardware/intel/gpu/mali-gralloc/intel/

LOCAL_SRC_FILES := hwcomposer_module.c \
	hwcomposer.cpp \
	hwcomposer_video.cpp \
	dcc-hal.c

LOCAL_STATIC_LIBRARIES += libpphwc

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := hwcomposer.$(TARGET_BOARD_PLATFORM)

LOCAL_CFLAGS:= -DLOG_TAG=\"IMChwc\"
LOCAL_CFLAGS+= -DHWC_VSYNC_THREAD
#LOCAL_CFLAGS+= -DHWC_FAKE_VSYNC_THREAD
#LOCAL_CFLAGS+= -DLOG_NDEBUG=0

include $(BUILD_SHARED_LIBRARY)

# Build the manual test programs.
#include $(call all-makefiles-under, $(LOCAL_PATH))

## FEAT_INTEL_HWC == true
#endif
