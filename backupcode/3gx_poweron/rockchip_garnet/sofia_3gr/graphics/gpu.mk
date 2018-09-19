# Copyright (C) 2013, 2014 Intel Mobile Communications GmbH# Copyright (C) 2011 The Android Open-Source Project
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
# Oct 20, 2013: Initial Creation for Sofia3G
# May 27 2014: IMC: add IMC test tools for engineering build

PRODUCT_COPY_FILES += \
  $(LOCAL_PATH)/gralloc.sofia3g.so:system/vendor/lib/hw/gralloc.$(TARGET_BOARD_PLATFORM).so \
  $(LOCAL_PATH)/libGLES_mali.so:system/vendor/lib/egl/libGLES_mali.so \
  $(LOCAL_PATH)/hwcomposer.sofia3g.so:system/vendor/lib/hw/hwcomposer.$(TARGET_BOARD_PLATFORM).so \
  $(LOCAL_PATH)/libion.so:system/vendor/lib/libion.so
