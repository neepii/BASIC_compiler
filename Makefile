SRC := $(shell find ./src -name *.c)
FLAGS := -Wall -g 
EXEC_NAME := exec
.PHONY: all

$(EXEC_NAME): parse.o main.o
	gcc -o $@ $^ $(FLAGS)

parse.o: src/parse.c
	gcc -c src/parse.c $(FLAGS)

main.o: src/main.c
	gcc -c $< $(FLAGS)

clean: 
	rm -rf *.o exec