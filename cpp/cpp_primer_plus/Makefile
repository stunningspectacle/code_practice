TARGETS := $(patsubst %.cpp, %, $(wildcard *.cpp))

all: $(TARGETS)

.PHONY: clean
clean:
	@find -perm -o=x -type f | xargs rm -rf
