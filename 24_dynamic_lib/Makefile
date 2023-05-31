CC = gcc
CFLAGS = -g -Werror -Wall -Wextra
	
all: build clean

build: object_files lib
	$(CC) $(CFLAGS) calculator.c -o calculator -L. -lmyname

lib: object_files
	$(CC) $(CFLAGS) -shared calculator.o my_func.o -o libmyname.so

object_files:
	$(CC) $(CFLAGS) calculator.c -fPIC -c
	$(CC) $(CFLAGS) my_func.c -fPIC -c

clean:
	rm -rf *.o *.out	

run: calculator
	LD_LIBRARY_PATH=$PWD:$LD_LIBRARY_PATH ./calculator
