CC = gcc
CFLAGS = -g -Werror -Wall -Wextra
	
all: build clean

build: lib
	$(CC) $(CFLAGS) calculator.c -o calculator -ldl

lib: object_files
	$(CC) $(CFLAGS) -shared calculator.o get_nums.o show_menu.o case_minus.o case_plus.o case_multiple.o case_div.o -o libmyname.so

object_files:
	$(CC) $(CFLAGS) calculator.c -fPIC -c
	$(CC) $(CFLAGS) get_nums.c -fPIC -c
	$(CC) $(CFLAGS) show_menu.c -fPIC -c
	$(CC) $(CFLAGS) case_minus.c -fPIC -c
	$(CC) $(CFLAGS) case_plus.c -fPIC -c
	$(CC) $(CFLAGS) case_multiple.c -fPIC -c
	$(CC) $(CFLAGS) case_div.c -fPIC -c

clean:
	rm -rf *.o *.out	

# Пример запуска
run:
	./calculator show_menu
