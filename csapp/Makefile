TARGETS := $(patsubst %.c, %, $(wildcard *.c))

all: $(TARGETS)

.PHONY: clean
clean:
	@find -perm -o=x -type f | xargs rm -rf
