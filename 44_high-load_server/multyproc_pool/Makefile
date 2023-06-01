CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) task3.c -lpthread
	
clean:
	rm -rf *.o

run_sv:
	./a.out -s

run_cl:
	./a.out -c
