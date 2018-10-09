OBJ=../obj/
SRC=../src/
INCLUDE=-I $(SRC)
CXX=g++
CFLAGS=-Wall -Wextra -std=c++11 -c
LFLAGS=-static
MAKE_OBJ=$(CXX) $(CFLAGS) $(INCLUDE) $< -o $@
MAKE_EXE=$(CXX) $(LFLAGS) $^ -o $@

test:	test_state_machine test_tokenizer

test_state_machine: test_state_machine.exe
	./test_state_machine.exe

test_tokenizer: test_tokenizer.exe
	./test_tokenizer.exe

test_state_machine.exe:	$(OBJ)test_state_machine.o $(OBJ)token_state_machine.o
	$(MAKE_EXE)

test_tokenizer.exe:	$(OBJ)tokenizer.o $(OBJ)token_state_machine.o $(OBJ)test_tokenizer.o
	$(MAKE_EXE)

$(OBJ)test_state_machine.o:	test_state_machine.cpp $(SRC)token_state_machine.hpp testing.hpp
	$(MAKE_OBJ)

$(OBJ)token_state_machine.o:	$(SRC)token_state_machine.cpp $(SRC)token_state_machine.hpp
	$(MAKE_OBJ)

$(OBJ)test_tokenizer.o:	test_tokenizer.cpp $(SRC)token.hpp $(SRC)tokenizer.hpp $(SRC)token_state_machine.hpp testing.hpp
	$(MAKE_OBJ)

$(OBJ)tokenizer.o:	$(SRC)tokenizer.cpp $(SRC)tokenizer.hpp $(SRC)token.hpp
	$(MAKE_OBJ)