CC = gcc
CFLAGS = -g
	
all: build clean

build:
	sudo $(CC) $(CFLAGS) task1.c

clean:
	rm -rf *.o

run_sv:
	./a.out -s

run_cl:
	./a.out -c

# Можно указать ключ очереди -k 1, 2, 3... , тогда чат будет подключен к очереди по данному ключу
