TARGET := rom
CROSS_COMPILE := arm-zephyr-eabi-

V := 0
ifeq ($(V), 1)
Q = 
else
Q = @
endif

CC := $(CROSS_COMPILE)gcc
LD := $(CROSS_COMPILE)ld
OBJDUMP := $(CROSS_COMPILE)objdump
OBJCOPY := $(CROSS_COMPILE)objcopy

CFLAGS := -Os -g -Wall -Werror -std=c99 -mabi=aapcs -mthumb -mcpu=cortex-m7
LDFLAGS := -gc-sections

ROOT_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
OUT := $(ROOT_DIR)out

OBJS := $(patsubst %.c, $(OUT)/%.o, $(wildcard *.c)) \
		$(patsubst %.S, $(OUT)/%.o, $(wildcard *.S))

$(OUT)/$(TARGET).bin: $(OUT)/$(TARGET).elf
	$(Q)$(OBJCOPY) -O binary --gap-fill 0 $< $@

$(OUT)/$(TARGET).elf: $(OUT) $(OBJS)
	$(Q)$(LD) $(LDFLAGS) -T $(TARGET).ld -o $@ $(OBJS)
	$(Q)$(OBJDUMP) -D $@ > $(OUT)/$(TARGET).dump

$(OUT)/%.o: %.c
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<
	
$(OUT)/%.o: %.S
	$(Q)$(CC) $(CFLAGS) -c -o $@ $<

$(OUT):
	mkdir -p $@

clean:
	rm -rf $(OUT)
