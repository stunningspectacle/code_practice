# Copyright (C) 2014 Intel Mobile Communications GmbH
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
#

LOCAL_PATH := $(call my-dir)

# HAL module implemenation stored in
# hw/<POWERS_HARDWARE_MODULE_ID>.<ro.hardware>.so
include $(CLEAR_VARS)

LOCAL_MODULE:= power.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := liblog libcutils libdl
LOCAL_SRC_FILES := \
                   power.c

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),sofia_lte sflte_2))
LOCAL_CFLAGS += -DBOARD_REV_LTE
else
ifneq (,$(findstring sofia3g, $(TARGET_BOARD_PLATFORM)))
LOCAL_CFLAGS += -DBOARD_REV_3G
else
$(error  $(TARGET_BOARD_PLATFORM) is not supported)
endif
endif

include $(BUILD_SHARED_LIBRARY)
