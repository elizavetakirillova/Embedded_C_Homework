CC = gcc
CFLAGS = -fPIC -Wall -Werror -Wextra -g

all: build clean

build: object_file lib
	$(CC) calculator.o -o calculator -L. -lmy_func

lib: object_file
	ar rc libmy_func.a my_func.o
	
object_file:
	$(CC) $(CFLAGS) my_func.c -c
	$(CC) $(CFLAGS) calculator.c -c

clean:
	rm -rf *.o *.out
