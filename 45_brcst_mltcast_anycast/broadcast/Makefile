CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) br_sv.c -o br_sv
	$(CC) $(CFLAGS) br_cl.c -o br_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./br_sv &

run_cl:
	./br_cl
