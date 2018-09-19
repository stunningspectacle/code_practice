# Copyright (C) 2011-2013 Intel Mobile Communications GmbH
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


LOCAL_PATH := $(call my-dir)

# HAL module implementation stored in
# hw/<OVERLAY_HARDWARE_MODULE_ID>.<ro.product.board>.so
include $(CLEAR_VARS)

# headers (needed by e.g. camera)
LOCAL_COPY_HEADERS := gralloc_priv.h
LOCAL_COPY_HEADERS_TO := libgralloc
include $(BUILD_COPY_HEADERS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)

LOCAL_PRELINK_MODULE := false
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_SHARED_LIBRARIES := liblog libhardware libcutils libion libutils libGLESv1_CM
# libMali

LOCAL_SRC_FILES := 	\
	gralloc.cpp 	\
	mapper.cpp      \
	ion-hal.cpp

#framebuffer.cpp

LOCAL_MODULE_TAGS := optional
LOCAL_MODULE := gralloc.$(TARGET_BOARD_PLATFORM)

LOCAL_CFLAGS:= -DLOG_TAG=\"IMCGralloc\"
#LOCAL_CFLAGS+= -DLOG_NDEBUG=0

include $(BUILD_SHARED_LIBRARY)

