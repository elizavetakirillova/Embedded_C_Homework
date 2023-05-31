#include <stdio.h>
#include <string.h>
#include <dlfcn.h> 

void get_nums (int * firstNum, int * secondNum)
{
    	printf("Введите два числа, над которыми вы хотите совершить действия.\n");
    	printf("Введите число a: ");
    	scanf("%d", firstNum);
    	printf("Введите число b: ");
    	scanf("%d", secondNum);
}
