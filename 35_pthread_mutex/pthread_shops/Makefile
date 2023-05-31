CC = gcc
CFLAGS = -g
	
all: build clean

build: 
	$(CC) $(CFLAGS) pthread_shops.c -o pthread_shops -lpthread
	
clean:
	rm -rf *.o *.out

run:
	./pthread_shops
