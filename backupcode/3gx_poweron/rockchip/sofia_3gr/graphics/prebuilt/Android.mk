# Copyright (c) 2015 Intel Corporation
ifneq ($(filter sofia3g sofia3gr,$(TARGET_BOARD_PLATFORM)),)

LOCAL_PATH := $(call my-dir)

ifeq ($(HAL_AUTODETECT),true)

# This creates dangling symbolic links for graphic libraries into
# /system/vendor/lib/ pointing to /system/rt/gfx/.
# For the sofia 3gr mali450 chip, the content of /system/vendor/gfx/sofia_3gr/
# is bind-mounted into /system/rt/gfx/ by hald upon detection of the mali chip.

ifneq ($(TARGET_DEVICE),Sf3gr_sr_garnet)
# vendor/lib
SYMLINKS := lib/libsurfaceflinger.so lib/libEGL.so lib/libion.so
SYMLINK_PAIRS := $(foreach f,$(SYMLINKS),$(TARGET_OUT)/vendor/$(f):../../rt/gfx/$(f))
endif

# vendor/lib/hw && vendor/lib/egl
SYMLINKS := lib/hw/hwcomposer.$(TARGET_BOARD_PLATFORM).so lib/egl/libGLES_mali.so
SYMLINK_PAIRS += $(foreach f,$(SYMLINKS),$(TARGET_OUT)/vendor/$(f):../../../rt/gfx/$(f))
SYMLINK_PAIRS += $(TARGET_OUT)/vendor/lib/hw/gralloc.autodetect.so:../../../rt/gfx/lib/hw/gralloc.$(TARGET_BOARD_PLATFORM).so

# It seems libGLES_mali.so also needs to exist under system, make a symbolic link here to
SYMLINK_PAIRS += $(TARGET_OUT)/lib/egl/libGLES_mali.so:../../rt/gfx/lib/egl/libGLES_mali.so

SYMLINKS := \
	$(foreach item, $(SYMLINK_PAIRS), $(call word-colon, 1, $(item)))

$(SYMLINKS):
	$(hide) echo "Creating symbolic link on $(notdir $@)"
	$(eval PRV_TARGET := $(call word-colon, 2, $(filter $@:%, $(SYMLINK_PAIRS))))
	mkdir -p $(dir $@)
	mkdir -p $(dir $(dir $@)$(PRV_TARGET))
	touch $(dir $@)$(PRV_TARGET)
	ln -sf $(PRV_TARGET) $@

ALL_DEFAULT_INSTALLED_MODULES += $(SYMLINKS)

# Install files into /system/vendor/gfx/sofia_3gr/
PRV_MODULE_PATH := $(TARGET_OUT)/vendor/gfx/sofia_3gr

else # $(HAL_AUTODETECT)

# No auto-detection. Install files into /system/vendor/
PRV_MODULE_PATH := $(TARGET_OUT)/vendor

endif # $(HAL_AUTODETECT)

GFX_MODULES :=

ifneq ($(TARGET_DEVICE),Sf3gr_sr_garnet)

include $(CLEAR_VARS)
LOCAL_MODULE := lib/libsurfaceflinger.so
LOCAL_SRC_FILES := libsurfaceflinger.so
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(PRV_MODULE_PATH)
GFX_MODULES += $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := lib/libEGL.so
LOCAL_SRC_FILES := libEGL.so
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(PRV_MODULE_PATH)
GFX_MODULES += $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := lib/libion.so
LOCAL_SRC_FILES := libion.so
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(PRV_MODULE_PATH)
GFX_MODULES += $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

endif

include $(CLEAR_VARS)
LOCAL_MODULE := lib/hw/gralloc.$(TARGET_BOARD_PLATFORM).so
ifeq ($(TARGET_DEVICE),Sf3gr_sr_garnet)
LOCAL_SRC_FILES := Sf3gr_sr_garnet/gralloc.sofia3g.so
else
LOCAL_SRC_FILES := gralloc.sofia3g.so
endif
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(PRV_MODULE_PATH)
GFX_MODULES += $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := lib/hw/hwcomposer.$(TARGET_BOARD_PLATFORM).so
ifeq ($(TARGET_DEVICE),Sf3gr_sr_garnet)
LOCAL_SRC_FILES := Sf3gr_sr_garnet/hwcomposer.sofia3g.so
else
LOCAL_SRC_FILES := hwcomposer.sofia3g.so
endif
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(PRV_MODULE_PATH)
GFX_MODULES += $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := lib/egl/libGLES_mali.so
ifeq ($(TARGET_DEVICE),Sf3gr_sr_garnet)
LOCAL_SRC_FILES := Sf3gr_sr_garnet/libGLES_mali.so
else
LOCAL_SRC_FILES := libGLES_mali.so
endif
LOCAL_MODULE_OWNER := intel
LOCAL_MODULE_CLASS := SHARED_LIBRARIES
LOCAL_MODULE_PATH := $(PRV_MODULE_PATH)
GFX_MODULES += $(LOCAL_MODULE)
include $(BUILD_PREBUILT)

include $(CLEAR_VARS)
LOCAL_MODULE := sofia_3gr_gfx_prebuilts
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_OWNER := intel
LOCAL_REQUIRED_MODULES := $(GFX_MODULES)
include $(BUILD_PHONY_PACKAGE)

endif
