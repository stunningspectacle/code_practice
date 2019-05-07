TARGETS := $(patsubst %.c, %, $(wildcard *.c))
all: $(TARGETS)

clean:
	find -perm -o=x -type f | xargs rm

