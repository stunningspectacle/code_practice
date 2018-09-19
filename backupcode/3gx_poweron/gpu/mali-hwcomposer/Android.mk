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
# Select based on TARGET_BOARD_PLATFORM
#ifeq ($(TARGET_BOARD_PLATFORM),sflte_2)
#include $(LOCAL_PATH)/arm/Android.mk
#else
ifeq ($(TARGET_BOARD_PLATFORM),sofia_lte)
include $(LOCAL_PATH)/intel/Android.mk
endif
#endif
