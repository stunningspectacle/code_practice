TARGET := main
SRC := $(wildcard *.cpp)
HEADER := $(wildcard *.h)

$(TARGET): $(SRC) $(HEADER)
	g++ -o $@ $(SRC) $(HEADER)

.PHONY: clean
clean:
	@find -perm -o=x -type f | xargs rm -rf 

