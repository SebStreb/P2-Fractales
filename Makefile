CC = gcc
CFLAGS = -Wall -W
LDFLAGS = -lm -L/usr/local/lib -lSDL

all: fractal

launch: main
	@echo 'Launching program'
	@./fractal

testing: tests
	@echo 'Launching tests'
	@./tests

fractal: main.o lib
	@echo 'Making exec'
	@$(CC) -o fractal main.o libfractal/libfractal.a $(FLAGS) $(LDFLAGS)

main.o: main.c
	@echo 'Building main'
	@$(CC) -c -o main.o main.c $(FLAGS)

tests: test/tests.o lib
	@echo 'Making tests'
	@$(CC) -o tests test/tests.o libfractal/libfractal.a $(FLAGS) $(LDFLAGS)

test/tests.o: test/tests.c
	@echo 'Building tests'
	@$(CC) -c -o test/tests.o test/tests.c $(FLAGS)

lib:
	@echo 'Building lib'
	@cd libfractal && $(MAKE)

.PHONY: lib clean

clean:
	@echo 'Removing files'
	@rm -rf fractal tests *.o libfractal/*.o libfractal/*.a test/*.o
