#include <stdio.h>

void get_nums (int * firstNum, int * secondNum)
{
    	printf("Введите два числа, над которыми вы хотите совершить действия.\n");
    	printf("Введите число a: ");
    	scanf("%d", firstNum);
    	printf("Введите число b: ");
    	scanf("%d", secondNum);
}

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

void case_minus (int * firstNum, int * secondNum, int * result)
{
    	* result = (* firstNum) - (* secondNum);
}

void case_plus (int * firstNum, int * secondNum, int * result)
{
    	* result = (* firstNum) + (* secondNum);
}

void case_multiple (int * firstNum, int * secondNum, int * result)
{
    	* result = (* firstNum) * (* secondNum);
}

void case_div (int * firstNum, int * secondNum, int *result)
{
    	* result = (* firstNum) / (* secondNum);
}

void put_result (int result)
{
    	printf("\nВаш результат:\n");
    	printf("%d\n", result);
}
