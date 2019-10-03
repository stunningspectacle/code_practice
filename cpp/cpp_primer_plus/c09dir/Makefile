TARGET := myhello
CPP_SRC := $(wildcard *.cpp)
HEADERS := $(wildcard *.h)
C_OBJS := $(patsubst %.c, %.o, $(wildcard *.c))

$(TARGET): $(CPP_SRC) $(C_OBJS) $(HEADERS)
	g++ -o $@ $(CPP_SRC) $(C_OBJS)

%.o: %.c
	gcc -c -o $@ $<

.PHONY: clean
clean:
	@find -perm -o=x -type f | xargs rm -rf
	@rm -rf *.o
