CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) str_UNIX_sv.c -o str_UNIX_sv
	$(CC) $(CFLAGS) str_UNIX_cl.c -o str_UNIX_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./str_UNIX_sv &

run_cl:
	./str_UNIX_cl
