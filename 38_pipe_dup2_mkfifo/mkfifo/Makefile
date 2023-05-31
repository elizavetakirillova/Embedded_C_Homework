CC = gcc
CFLAGS = -g
	
all: build clean

build: 
	$(CC) $(CFLAGS) mkfifo_srv.c -o mkfifo_srv
	$(CC) $(CFLAGS) mkfifo_cl.c -o mkfifo_cl
	
clean:
	rm -rf *.o *.out

run_srv:
	./mkfifo_srv &
run_cl:
	./mkfifo_cl &

