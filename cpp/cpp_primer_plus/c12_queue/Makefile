queue: $(wildcard *.cpp)
	g++ -o $@ $(wildcard *.cpp)

.PHONY: clean
clean:
	@find -perm -o=x -type f | xargs rm -rf
