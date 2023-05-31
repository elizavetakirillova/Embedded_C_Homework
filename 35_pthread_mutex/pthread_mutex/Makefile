CC = gcc
CFLAGS = -g -Werror -Wall -Wextra
	
all: build clean

build: 
	$(CC) $(CFLAGS) pthread_mutex.c -o pthread_mutex -lpthread
	
clean:
	rm -rf *.o *.out

run:
	./pthread_mutex
