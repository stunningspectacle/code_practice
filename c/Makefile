#TARGETS := search print_env test macro execve
TARGET := mix
OBJS := $(patsubst %.c, %.o, $(wildcard *.c))
SRC := $(wildcard *.c)

LD := ld

$(TARGET): $(OBJS)
	@echo $(OBJS)
	@echo $@
	@echo $^
	$(LD) -o $@ $^
$(OBJS): $(SRC)


clean:
	rm -rf $(OBJS)

