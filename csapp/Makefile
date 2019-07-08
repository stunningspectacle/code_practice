TARGETS := $(patsubst %.c, %, $(wildcard *.c))

CFLAGS += -O2

all: $(TARGETS)

.PHONY: clean dump
clean:
	@find -perm -o=x -type f | xargs rm -rf

dump: $(TARGETS)
	objdump -S $< > $<.obj


