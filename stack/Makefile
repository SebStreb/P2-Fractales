CC = gcc
AR = ar
CFLAGS = -g -Wall -W -std=c99
LDFLAGS =
SRC = stack.c
OBJ = $(SRC:.c=.o)
LIBFILE = libstack.a

all: $(SRC) $(LIBFILE)

$(LIBFILE): $(OBJ)
	@$(AR) r $@ $(OBJ)

*.o:
	@$(CC) -o $@ -c $< $(CFLAGS)
