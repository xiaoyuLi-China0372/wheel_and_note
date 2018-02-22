all: run
.PHONY: all

run: src.o
	gcc -o $@ $^
%.o: %.c
	gcc -c $^ -o $@

clean:
	-rm *.o run
.PHONY: clean
