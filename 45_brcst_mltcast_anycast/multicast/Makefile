CC = gcc
CFLAGS = -g
	
all: build clean

build:
	$(CC) $(CFLAGS) ml_sv.c -o ml_sv
	$(CC) $(CFLAGS) ml_cl.c -o ml_cl
	
clean:
	rm -rf *.o *.out

run_sv:
	./ml_sv &

run_cl:
	./ml_cl
