CC = gcc
CFLAGS = -g
	
all: build clean

build: 
	$(CC) $(CFLAGS) losCurses2.c -o outBuild -lncurses
	
clean:
	rm -rf *.o *.out

run:
	sudo  valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --show-below-main=yes --keep-debuginfo=yes --log-file=loserror.txt ./outBuild
