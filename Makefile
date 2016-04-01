CC = gcc
CFLAGS = -g -Wall -W -I$(HOME)/local/include
LDFLAGS = -lm -L/usr/local/lib -L$(HOME)/local/lib -lSDL -lcunit -pthread
all: fractal

testing: tests
	@echo 'Launching tests'
	@./tests

fractal: main.o libfractal/libfractal.a
	@echo 'Making exec'
	@$(CC) -o fractal main.o libfractal/libfractal.a $(CFLAGS) $(LDFLAGS)

main.o: main.c
	@echo 'Building main'
	@$(CC) -c -o main.o main.c $(CFLAGS)

tests: test/tests.o libfractal/libfractal.a
	@echo 'Making tests'
	@$(CC) -o tests test/tests.o libfractal/libfractal.a $(CFLAGS) $(LDFLAGS)

test/tests.o: test/tests.c
	@echo 'Building tests'
	@$(CC) -c -o test/tests.o test/tests.c $(CFLAGS)

libfractal/libfractal.a:
	@echo 'Building lib'
	@cd libfractal && $(MAKE)

.PHONY: clean

clean:
	@echo 'Removing files'
	@rm -rf fractal tests *.o libfractal/*.o libfractal/*.a test/*.o *.bmp
