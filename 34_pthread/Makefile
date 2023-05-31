CC = gcc
CFLAGS = -g -Werror -Wall -Wextra
	
all: build clean

build: 
	$(CC) $(CFLAGS) pthread.c -o pthread -lpthread
	
clean:
	rm -rf *.o *.out

run:
	./pthread
