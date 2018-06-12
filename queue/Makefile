all: run
.PHONY: all

C_FLAGS = -g -std=c99

BUILD_DIR = .
SRC_DIR = .
C_SRCS = $(wildcard $(foreach n,$(SRC_DIR),$(n)/*.c))
C_OBJS = $(addprefix $(BUILD_DIR)/, ${C_SRCS:.c=.o})
run: $(C_OBJS)
	gcc -o $@ $^
%.o: %.c
	gcc $(C_FLAGS) -c $^ -o $@

clean:
	-rm *.o run
.PHONY: clean
