TARGETS := $(patsubst %.c, %, $(wildcard *.c))
#CFLAGS += -fPIC

all: $(TARGETS)

.PHONY:
clean:
	@find -perm -o=x -type f | xargs rm -rf
