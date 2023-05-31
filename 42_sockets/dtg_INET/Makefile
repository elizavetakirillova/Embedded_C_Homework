CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) dtg_INET_sv.c -o dtg_INET_sv
	$(CC) $(CFLAGS) dtg_INET_cl.c -o dtg_INET_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./dtg_INET_sv &

run_cl:
	./dtg_INET_cl
