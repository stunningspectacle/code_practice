TARGETS := $(patsubst %.cpp, %, $(wildcard *.cpp))

CPPFLAGS += -pthread

all: $(TARGETS)

.PHONY: clean
clean:
	@find -perm -o=x -type f | xargs rm -rf
