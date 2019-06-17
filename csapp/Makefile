TARGETS := $(patsubst %.c, %, $(wildcard *.c))
all: $(TARGETS)

.PHONY:
clean:
	@find -perm -o=x -type f | xargs rm -rf
