# Copyright (C) 2015 Intel Mobile Communications GmbH
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

# Only use precompiled binaries if mali-dp500 source is not present
ifeq ($(wildcard  $(LOCAL_PATH)/../PRIVATE/mali-dp500_src/driver/user/Android.mk),)

include $(CLEAR_VARS)
LOCAL_MODULE := hwcomposer.$(TARGET_BOARD_PLATFORM)
LOCAL_SRC_FILES := hwcomposer.$(TARGET_BOARD_PLATFORM).so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
include $(BUILD_PREBUILT)

ifeq ($(TARGET_BUILD_TYPE),debug)
include $(CLEAR_VARS)
LOCAL_MODULE := libadf_malidp
LOCAL_SRC_FILES := $(LOCAL_PATH)/test/lib/libadf_malidp.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := libmali_utf
LOCAL_SRC_FILES := $(LOCAL_PATH)/test/lib/libmali_utf.so
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_SUFFIX := .so
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := malidp_integration_tests
LOCAL_SRC_FILES := $(LOCAL_PATH)/test/malidp_integration_tests
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := malidp_cv_unit
LOCAL_SRC_FILES := $(LOCAL_PATH)/test/malidp_cv_unit
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_CLASS := EXECUTABLES
LOCAL_MODULE_PATH := $(TARGET_OUT_EXECUTABLES)
include $(BUILD_PREBUILT)

# [TODO] include the .sh file as well
endif

# Use mali-dp500 source files to generate the binaries
else
include $(LOCAL_PATH)/../PRIVATE/mali-dp500_src/driver/user/Android.mk
ifeq ($(TARGET_BUILD_TYPE),debug)
include $(LOCAL_PATH)/../PRIVATE/mali-dp500_src/driver/tests/Android.mk
endif
endif
