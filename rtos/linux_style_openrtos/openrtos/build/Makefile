
PROJECT_CSRC := $(LOCAL_CSRC)
PROJECT_ASRC := $(LOCAL_ASRC)
PROJECT_CFLAGS := $(LOCAL_CFLAGS)
PROJECT_VPATH :=
TARGET_PLATFORM :=

TARGET_NAME := out
OUT_FOLDER := ..\bin

ifneq ($(TARGET_PLATFORM), clanton)
LD_SCRIPT_ORIG			:= cmd.lds
else
LD_SCRIPT_ORIG			:= clanton.lds
endif

LD_SCRIPT_PREPROCESSED	:= $(TARGET_PLATFORM).preprocessed.lds
LD_HEADERS := -I..\..\kernel_add_ons\include -I..\..\kernel_add_ons\os_api\openrtos

include $(subst \,/,$(BASE_DIR))/BuildTools/ish_make/common.rtos.mk
include $(PROJ_MK)

include defs.mk
include tools.mk
include rules.mk
include flags.mk



TARGET := $(OUT_FOLDER)\$(TARGET_NAME)

VPATH = ..\:$(PROJECT_VPATH)

vpath %.c ..\ 
vpath %.S ..\ 

#TODO - string.S should be used from ISH code, but for standalone compilation - we should take the local one
#TODO - is i2c code in the correct position?

A_SRCS :=       OpenRTOS_kernel\Source\portable\GCC\IA32_flat\portASM.S \
                OpenRTOS_kernel\Platform\Common\string.S \
                OpenRTOS_kernel\Platform\x86\Common\startup.S \
                OpenRTOS_kernel\Platform\x86\Common\exceptionASM.S \
                OpenRTOS_kernel\Platform\x86\Common\mmuASM.S

C_SRCS :=       OpenRTOS_kernel\Source\croutine.c \
                OpenRTOS_kernel\Source\event_groups.c \
                OpenRTOS_kernel\Source\list.c \
                OpenRTOS_kernel\Source\queue.c \
                OpenRTOS_kernel\Source\tasks.c \
                OpenRTOS_kernel\Source\timers.c \
                OpenRTOS_kernel\Source\portable\MemMang\heap_4.c \
                OpenRTOS_kernel\Source\portable\GCC\IA32_flat\port.c \
                OpenRTOS_kernel\Source\portable\GCC\IA32_flat\partest.c \
                OpenRTOS_kernel\Platform\x86\Common\ia32_interrupts.c \
                OpenRTOS_kernel\Platform\x86\Common\exception.c \
                OpenRTOS_kernel\Platform\x86\Common\fpu.c \
                OpenRTOS_kernel\Platform\x86\Common\gdt.c \
                OpenRTOS_kernel\Platform\x86\Common\init.c \
                OpenRTOS_kernel\Platform\x86\Common\rtos_trc_recorder_wrapper.c \
                OpenRTOS_kernel\Platform\x86\$(TARGET_PLATFORM)\system.c \
                ..\services\service_gpio.c \
                ..\services\service_timer.c \
                libish\printf.c \
                libish\memcpy.c \
                libish\syslog.c \
                libish\trace_filter.c \
                libish\secure_mem_funcs.c \
                libish\heci_client_number.c 

ifeq ($(ISH_CONFIG_SUPPORT_RTOS_TRACE_RECORDER),1)

C_SRCS += $(TRACE_REC_LIB_SRC)\trcKernelPort.c \
          $(TRACE_REC_LIB_SRC)\trcSnapshotRecorder.c \
          $(TRACE_REC_LIB_SRC)\trcStreamingRecorder.c \
          $(TRACE_REC_LIB_SRC)\streamports\HECI\trcStreamingPort.c
endif

ifeq ($(ISH_CONFIG_SUPPORT_PAGING),1)
C_SRCS := $(C_SRCS) OpenRTOS_kernel\Platform\x86\Common\mmu.c OpenRTOS_kernel\Platform\x86\Common\mmu\paging.c
endif

ifeq ($(BULLSEYE_EN), 1)
C_SRCS := $(C_SRCS) BullsEye\libcov-userDefined-small.c BullsEye\ZBullseyeDump.c
CC := $(BULLSEYE_DIR)\covc.exe --no-banner --no-lib $(CC) 
endif

ifeq ($(FW_TEST), 1)
include $(subst \,/,$(BASE_DIR))/Src/apps/fw_test/fw_test.mk
include $(subst \,/,$(BASE_DIR))/Src/apps/fw_test/kernel_space/fw_testk.mk
C_SRCS := $(C_SRCS) $(FWTESTSRC) $(FWTESTSRCK)
CFLAGS += $(FWTESTINCPATH) $(FWTESTCFLAG)
endif

ifeq ($(TARGET_PLATFORM), clanton)
        A_SRCS := $(A_SRCS) OpenRTOS_kernel\Platform\x86\Common\io.S
endif


CFLAGS += -I..\openRTOS_kernel\Platform\x86\Common \
          -I..\openRTOS_kernel\Platform\x86\$(TARGET_PLATFORM) \
          -I..\..\include \
          -I..\..\include\services \
          -I..\..\include_ext\POSIX \
          -I..\..\include_ext \
          -I..\..\include\drivers \
          -I..\..\include\bsp_config\$(CUR_BSP)

A_SRCS := $(A_SRCS) $(PROJECT_ASRC)

KADDONS_SSRC := $(filter %.s, $($(KADDONS_CSRC)))
KADDONS_CSRC := $(filter %.c, $($(KADDONS_CSRC)))
ARCHIVE_KADDONS_OBJS := $(foreach obj,$(subst .c,.o,$(KADDONS_CSRC)) $(subst .s,.o,$(KADDONS_SSRC)),$(obj))
ARCHIVE_KADDONS_CATAS := $(foreach cata,$(subst .o,.cata2,$(ARCHIVE_KADDONS_OBJS)),$(cata))

SERVICE_PROJECT_SSRCS := $(filter %.s, $(SERVICE_PROJECT_CSRC))
SERVICE_PROJECT_CSRCS := $(filter %.c, $(SERVICE_PROJECT_CSRC))
SERVICE_PROJECT_ASRCS := $(SERVICE_PROJECT_ASRC)
PROJECT_SSRCS := $(filter %.s, $(PROJECT_CSRC))
PROJECT_CSRCS := $(filter %.c, $(PROJECT_CSRC))
PROJECT_ASRCS := $(PROJECT_ASRC)
SENSOR_PROJECT_SSRCS := $(filter %.s, $(SENSOR_PROJECT_CSRC))
SENSOR_PROJECT_CSRCS := $(filter %.c, $(SENSOR_PROJECT_CSRC))
SENSOR_PROJECT_ASRCS := $(SENSOR_PROJECT_ASRC)
ARCHIVE_PROJECT_OBJS := $(foreach obj,$(subst .S,.o,$(SENSOR_PROJECT_ASRCS)) $(subst .c,.o,$(SENSOR_PROJECT_CSRCS)) $(subst .s,.o,$(SENSOR_PROJECT_SSRCS)),$(obj))
ARCHIVE_PROJECT_CATAS := $(foreach cata,$(subst .o,.cata2,$(ARCHIVE_PROJECT_OBJS)),$(cata))

ifeq ($(PREBUILT),1)
C_SRCS := $(C_SRCS) $(SERVICE_PROJECT_CSRC) $(PROJECT_CSRC)
include $(PROJECT_BASE)/prebuilt/prebuilt_objs.mk
else
C_SRCS := $(C_SRCS) $(KADDONS_CSRC) $(SERVICE_PROJECT_CSRC) $(SENSOR_PROJECT_CSRC) $(PROJECT_CSRC)
endif
CFLAGS += $(PROJECT_CFLAGS) $(KADDONS_CFLAGS)

define INCLUDE_MAKE =
-include $(OUT_FOLDER)/.$(basename $(notdir $(1))).$(2).d

endef

# include the autogenerated rules - for handling header file dependencies
S_SRCS := $(filter %.s, $(C_SRCS))
C_SRCS := $(filter %.c, $(C_SRCS))

$(foreach asm, $(A_SRCS), $(eval $(call INCLUDE_MAKE, $(asm),s)))
$(foreach src, $(C_SRCS), $(eval $(call INCLUDE_MAKE, $(src),c)))

RELATED_OBJS := $(foreach obj,$(subst .S,.o,$(A_SRCS)) $(subst .c,.o,$(C_SRCS)) $(subst .s,.o,$(S_SRCS)), $(obj))
OBJS := $(foreach obj,$(subst .S,.o,$(A_SRCS)) $(subst .c,.o,$(C_SRCS)) $(subst .s,.o,$(S_SRCS)),$(OUT_FOLDER)/$(obj))
ifneq ($(PREBUILT),1)
ABSOLUTE_OBJS := $(OBJS)
else
ABSOLUTE_OBJS := $(foreach obj,$(PREBUILT_OBJS),$(OUT_FOLDER)/$(obj))
endif

OUTPUT_DIR_TREE := $(subst /,\,$(sort $(foreach obj,$(OBJS),$(dir $(obj)))))

LIBS_DIR := lib/gcc/i486-elf/8.3.0
LIBS := -lgcc --start-group --whole-archive -lishmath --end-group

all:    build_directories $(TARGET).bin

$(TARGET).bin: $(TARGET).elf
	@echo creating $(notdir $@)
	@$(OBJCOPY) -O binary $< $@


$(LD_SCRIPT_PREPROCESSED): $(LD_SCRIPT_ORIG)
	@echo preprocessing lds file 
ifneq ($(PREBUILT),1)
	$(RM) /f $(LD_SCRIPT_PREPROCESSED)
	@$(CC) -x assembler-with-cpp -nostdinc -undef -Wundef -E $(LOCAL_FLAGS) $(CFLAGS) -P $(LD_SCRIPT_ORIG) $(LD_HEADERS) -o $(LD_SCRIPT_PREPROCESSED)
endif

LD_SCRIPT_OBJ_LIST := $(TARGET_PLATFORM).objlist.lds
LD_OBJS := INPUT($(ABSOLUTE_OBJS))
AR_SCRIPT := $(TARGET_PLATFORM).ars

$(TARGET).elf: $(LD_SCRIPT_PREPROCESSED) $(OBJS) 
	@echo creating $(notdir $@) with $(LIBS)
ifneq ($(PREBUILT),1)
	@echo GEN_LIBDIR=$(LIBS_DIR)  > $(OUT_FOLDER)/fdk_lib_gen.mk
	@echo GEN_LIB=$(LIBS)  >> $(OUT_FOLDER)/fdk_lib_gen.mk
	@echo CUR_BSP=$(CUR_BSP) >> $(OUT_FOLDER)/fdk_lib_gen.mk
	@echo GEN_CC_OPT=$(filter-out -Wunused-parameter -Wswitch-default -I% -include %autoconf.h,$(CFLAGS)) >> $(OUT_FOLDER)/fdk_lib_gen.mk
	@echo GEN_LK_OPT=-dT $(LD_SCRIPT_PREPROCESSED) $(LDFLAGS) --build-id=none >> $(OUT_FOLDER)/fdk_lib_gen.mk
	@echo GEN_OC_OPT=-O binary  >> $(OUT_FOLDER)/fdk_lib_gen.mk
	@echo copying files for next prebuilt using
	@$(SEVENZ) a -t7z -mx9 -r $(PROJECT_BASE)/prebuilt/$(PROJECT_FULL_NAME)_headers.7z $(KADDONS_INCLUDE_FILE)
	@pushd $(OUT_FOLDER) & dir & $(SEVENZ) a -t7z -mx9 -r $(PROJECT_BASE)/prebuilt/$(PROJECT_FULL_NAME)_app_objs.7z $(ARCHIVE_PROJECT_OBJS) $(ARCHIVE_PROJECT_CATAS) & popd
	@echo PREBUILT_OBJS := $(RELATED_OBJS) > $(PROJECT_BASE)/prebuilt/prebuilt_objs.mk
	@pushd $(OUT_FOLDER) & dir & $(SEVENZ) a -t7z -mx9 -r $(PROJECT_BASE)/prebuilt/$(PROJECT_FULL_NAME)_kaddons_objs.7z $(ARCHIVE_KADDONS_OBJS) $(ARCHIVE_KADDONS_CATAS) & popd
	@copy /y $(LD_SCRIPT_PREPROCESSED) $(PROJECT_BASE)\prebuilt\$(LD_SCRIPT_PREPROCESSED)
	@copy /y $(OUT_FOLDER)\fdk_lib_gen.mk $(PROJECT_BASE)\prebuilt\fdk_lib_gen.mk
endif
	@$(file > $(LD_SCRIPT_OBJ_LIST),$(LD_OBJS))
	@$(LD) -dT $(LD_SCRIPT_PREPROCESSED) $(LDFLAGS) --build-id=none -o $@ -Map $(TARGET).map -T $(LD_SCRIPT_PREPROCESSED) -T $(LD_SCRIPT_OBJ_LIST) $(foreach dir, $(LIBS_DIR),-L$(GCC_DIR)/$(dir)) -L$(OUTPUT_FOLDER_EXTRA) $(LIBS)
	@$(file > $(AR_SCRIPT),CREATE $(OUT_FOLDER)\libsensor_core.a)
	@$(file >> $(AR_SCRIPT),ADDMOD $(filter-out %sensor_plat.o, $(ABSOLUTE_OBJS)))
	@$(file >> $(AR_SCRIPT),SAVE)
	@$(file >> $(AR_SCRIPT),END)
	@$(AR) -M < $(AR_SCRIPT)
	@copy /y $(LD_SCRIPT_PREPROCESSED) $(OUT_FOLDER)\$(LD_SCRIPT_PREPROCESSED)
	@$(RM) $(LD_SCRIPT_PREPROCESSED) $(LD_SCRIPT_OBJ_LIST) $(AR_SCRIPT)
	@$(OBJDUMP) -S $@ > $(TARGET).lst

$(OBJS): $(firstword $(MAKEFILE_LIST)) $(wildcard *.mk)

rwildcard=$(wildcard $1$2) $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2))

ifneq ($(INCLUDE_PATH),)
CFLAGS += $(foreach inc_path,$(INCLUDE_PATH), -I"$(inc_path)")
endif

CFLAGS += $(EXTRA_CFLAGS)

ifneq ($(APPS_SRC_RECURSIVE),)
APPS_SRC_DIR := $(subst \,/,$(APPS_SRC_RECURSIVE))
##APPS_SRC_FILE := $(foreach n,$(APPS_SRC_DIR) , $(wildcard $(n)/*.c))
APPS_SRC_FILE := $(filter %.c %.C, $(foreach n,$(call rwildcard, $(APPS_SRC_RECURSIVE)/,*.c) , $(n)))

$(foreach src, $(APPS_SRC_FILE), $(eval $(call INCLUDE_MAKE, $(src),c)))

APPS_OBJS := $(foreach obj, $(subst .c,.o,$(APPS_SRC_FILE)) ,$(notdir $(obj)))
FILEPATH := $(sort $(dir $(foreach src_path,$(APPS_SRC_FILE), $(call rwildcard,$(src_path),*/) ) ))
VPATH:= $(FILEPATH)

$(APPS_OBJS): %.o: %.c
	@echo application compiling [$(notdir $<)]
	@$(CC) $(CFLAGS) $(DEP_FLAGS) $(OUT_FOLDER)\.$(notdir $<).d -c $< -o $(OUT_FOLDER)\$@

ifneq (,$(APPSLIB_OUT))
$(APPSLIB_OUT): $(APPS_OBJS)
	@echo application archiving [$(notdir $(APPSLIB_OUT))]
	@$(AR) cr $@ $(foreach filename,$^, $(OUT_FOLDER)/$(filename))
endif
endif


apps: $(APPS_OBJS) $(APPSLIB_OUT)

# subst '/' because RM is currently simply a del, so slashes must be windows oriented...
CLEAN_LIST := $(subst /,\, $(OBJS) $(OUT_FOLDER)/*.su $(TARGET).elf $(TARGET).bin $(TARGET).map $(TARGET_PATCHED) $(OUT_FOLDER)\.*.d)

FORCE:
	
    
build_directories: FORCE
	-@$(MD) $(OUTPUT_DIR_TREE) >nul 2>&1 || (exit 0)

.PHONY:	clean
clean:
	@echo cleaning...
	@$(RM) $(CLEAN_LIST) data_pad *cscope.* tags

.PHONY:	tags
tags:
	find . -name '*.[chS]' > cscope.files
	find ../include -name '*.h' >> cscope.files
	find ../include_ext -name '*.h' >> cscope.files
	find ../libish -name '*.[cS]' >> cscope.files
	cscope -k -q -b
	xargs ctags < cscope.files
