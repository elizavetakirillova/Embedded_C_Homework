CC = gcc
CFLAGS = -g -lpthread
	
all: build clean

build:
	$(CC) task3.c $(CFLAGS)
	
clean:
	rm -rf *.o

run_sv:
	sudo ./a.out -s

run_cl:
	sudo ./a.out -c

# sudo ufw allow 8000
# sudo ufw allow 8001
