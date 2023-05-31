CC = gcc
CFLAGS = -g
	
all: build clean

build: 
	$(CC) $(CFLAGS) dup2.c -o dup2
	
clean:
	rm -rf *.o *.out

run:
	./dup2 ls wc
