all: run
.PHONY: all

BUILD_DIR = .
SRC_DIR = .
C_SRCS = $(wildcard $(foreach n,$(SRC_DIR),$(n)/*.c))
C_OBJS = $(addprefix $(BUILD_DIR)/, ${C_SRCS:.c=.o})
run: $(C_OBJS)
	gcc -o $@ $^
%.o: %.c
	gcc -g -c $^ -o $@

clean:
	-rm *.o run
.PHONY: clean
