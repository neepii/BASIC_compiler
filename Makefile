SRC := $(shell find src/ -regex ".*\.c")
OBJ := $(patsubst src/%.c, %.o, $(SRC))
FLAGS := -Wall -g
EXEC_NAME := exec

.PHONY: all
all: $(EXEC_NAME)

$(EXEC_NAME): $(OBJ)
	gcc -o $@ $^ $(FLAGS)

%.o: src/%.c
	gcc -c $^ $(FLAGS)

clean: 
	rm -rf *.o exec