CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) dtg_UNIX_sv.c -o dtg_UNIX_sv
	$(CC) $(CFLAGS) dtg_UNIX_cl.c -o dtg_UNIX_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./dtg_UNIX_sv &

run_cl:
	./dtg_UNIX_cl
