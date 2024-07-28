SRC := $(shell find src/ -regex ".*\.c")
OBJ := $(patsubst src/%.c, build/%.o, $(SRC))
FLAGS := -Wall -g
EXEC_NAME := exec

all: $(EXEC_NAME)

$(EXEC_NAME): $(OBJ)
	gcc -o $@ $^ $(FLAGS)

build/%.o: src/%.c
	gcc -c $^ -o $@ $(FLAGS)

clean: 
	rm -rf build/*.o exec