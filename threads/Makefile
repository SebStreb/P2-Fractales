CC = gcc
AR = ar
CFLAGS = -g -Wall -W
LDFLAGS =
SRC = threads.c
OBJ = $(SRC:.c=.o)
LIBFILE = libthread.a

all: $(SRC) $(LIBFILE)

$(LIBFILE): $(OBJ)
	@$(AR) r $@ $(OBJ)

*.o:
	@$(CC) -o $@ -c $< $(CFLAGS)
