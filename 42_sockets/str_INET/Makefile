CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) str_INET_sv.c -o str_INET_sv
	$(CC) $(CFLAGS) str_INET_cl.c -o str_INET_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./str_INET_sv &

run_cl:
	./str_INET_cl
