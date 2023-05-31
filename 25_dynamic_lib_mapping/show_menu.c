#include <stdio.h>
#include <string.h>
#include <dlfcn.h> 

int show_menu (void)
{
	int answer;

    	printf("Выберите операцию:\n");
    	printf("1 a - b\n");
    	printf("2 a + b\n");
    	printf("3 a * b\n");
    	printf("4 a / b\n");

    	scanf(" %d", &answer);
	fflush(stdin);

	return answer;
}
