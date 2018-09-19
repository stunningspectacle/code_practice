# Copyright (C) 2013 Intel Mobile Communications GmbH
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

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += \
	$(LOCAL_PATH)/../ \
	$(LOCAL_PATH) \
	$(TOP)/hardware/intel/libgralloc \
	$(TOP)/external/kernel-headers/original

LOCAL_SHARED_LIBRARIES := libcutils libion libutils 


LOCAL_MODULE_TAGS := debug
LOCAL_MODULE := dcctests
LOCAL_SRC_FILES := dcc-tests.cpp \
		./../dcc-hal.c \
		./../../libgralloc/ion-hal.cpp


include $(BUILD_EXECUTABLE)

