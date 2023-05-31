CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) UDP_hdr_sv.c -o UDP_hdr_sv
	$(CC) $(CFLAGS) UDP_hdr_cl.c -o UDP_hdr_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./UDP_hdr_sv &

run_cl:
	./UDP_hdr_cl
