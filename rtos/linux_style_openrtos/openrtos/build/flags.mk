LDFLAGS :=		--defsym ISH_BOOT_START=$(ISH_BOOT_START) $(LD_ISH_CONFIG) -z muldefs	-gc-sections
LDFLAGS-UT =		-z muldefs -gc-sections

LANG_FLAGS :=		-std=gnu99 
ARCH_FLAGS :=		-march=pentium -mtune=i486
WARN_FLAGS :=		-Wno-unused-but-set-variable -Wall -Werror -Wswitch-default -Wunused-parameter -Wundef
OPT_FLAGS :=	 -fno-omit-frame-pointer -mno-accumulate-outgoing-args -ffunction-sections -fdata-sections	\
				-fno-builtin-printf -fno-builtin-sprintf 

ifeq ($(BULLSEYE_EN), 1)
CFLAGS += -Drestrict=__restrict__ -D_BULLSEYE_EN 
WARN_FLAGS :=		-Wno-unused-but-set-variable -Wall -Werror -Wswitch-default -Wunused-parameter -Wunused-function -Wundef \
					-Wno-error=unused-function -Wno-error=undef -Wno-error=unused-parameter
endif                         
                
                
ifeq ($(DEBUG_MODE),1)
DBG_FLAGS :=		-g2 -fstack-usage
else
DBG_FLAGS :=		-Os -fstack-usage
endif

LOCAL_FLAGS :=	-DISH_BOOT_MAJOR=$(ISH_BOOT_MAJOR) \
				-DISH_BOOT_MINOR=$(ISH_BOOT_MINOR) \
				-DSYSLOG_LOWEST=7 \
				-DOPENRTOS \
				-DUSING_ATTR
			
				
LOCAL_FLAGS :=	$(LOCAL_FLAGS) -DBOOT_START_ADDR=$(ISH_BOOT_START)
			
DEP_FLAGS :=		-MP -MD -MF

ifeq ($(ISH_CONFIG_SUPPORT_RTOS_TRACE_RECORDER),1)
TRACE_REC_LIB_SRC := 3rdparty\RecorderLib_OpenRTOS_v3.1.1
endif

# Windows support: make sure no double quote symbols left.
NORM_CFLAGS :=	-I..\openRTOS_kernel\Source\include -I..\openRTOS_kernel\Platform\Common\include -I..\openRTOS_kernel\Platform\Common\include\sys -I..\openRTOS_kernel\Source\portable\GCC\IA32_flat -I..\openRTOS_kernel\Platform\x86\Common -gdwarf-2 -fno-common -ffreestanding  -minline-all-stringops -fno-strict-aliasing  

ifeq ($(ISH_CONFIG_SUPPORT_RTOS_TRACE_RECORDER),1)
NORM_CFLAGS += -I..\$(TRACE_REC_LIB_SRC)\config -I..\$(TRACE_REC_LIB_SRC)\include -I..\$(TRACE_REC_LIB_SRC)\streamports\HECI\include
endif

ROM_CFLAGS =	$(NORM_CFLAGS) -DNEED_PRINTF -mregparm=0

CFLAGS :=		$(CFLAGS) $(LANG_FLAGS) $(ARCH_FLAGS) $(WARN_FLAGS) $(OPT_FLAGS)	\
				$(LOCAL_FLAGS) $(DBG_FLAGS) $(ROM_CFLAGS) -ffreestanding -minline-all-stringops
       
