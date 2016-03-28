CC = gcc
FLAGS = -Wall

all: main

launch: main
	@echo 'Launching program'
	@./main

main: main.o
	@echo 'Making exec'

main.o: main.c
	@echo 'Building main'
	@$(CC) -c -o main.o main.c $(FLAGS)

tests:
	@echo 'Making tests'

lib:
	@echo 'Building lib'
	@cd libfractal && $(MAKE)

.PHONY: lib clean

clean:
	@echo 'Removing files'
	@rm -rf main tests *.o libfractal/*.o libfractal/*.a
