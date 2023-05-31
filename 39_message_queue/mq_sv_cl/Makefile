CC = gcc
CFLAGS = -g
	
all: build clean

build: 
	$(CC) $(CFLAGS) mq_server.c -o mq_server -lrt
	$(CC) $(CFLAGS) mq_client.c -o mq_client -lrt
	$(CC) $(CFLAGS) mq_unlink.c -o mq_unlink -lrt
	$(CC) $(CFLAGS) mq_create.c -o mq_create -lrt
	
clean:
	rm -rf *.o *.out

run:
	./mq_create &
	./mq_server &
	./mq_client &
	
