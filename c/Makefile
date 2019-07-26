TARGETS := $(patsubst %.c, %, $(wildcard *.c))
CFLAGS += -Og

all: $(TARGETS)

clean:
	find -perm -o=x -type f | xargs rm

