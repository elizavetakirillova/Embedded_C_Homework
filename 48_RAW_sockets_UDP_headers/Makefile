CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) UDP_header_sv.c -o UDP_header_sv
	$(CC) $(CFLAGS) UDP_header_cl.c -o UDP_header_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./UDP_header_sv &

run_cl:
	sudo ./UDP_header_cl
