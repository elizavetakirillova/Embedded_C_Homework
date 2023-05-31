CC = gcc
CFLAGS = -g
	
all: build clean

build: 
	$(CC) $(CFLAGS) sph_server.c -o sph_server
	$(CC) $(CFLAGS) sph_client.c -o sph_client
	
clean:
	rm -rf *.o *.out

run_server:
	./sph_server &

run_client:
	./sph_client
