/* Елизавета Кириллова. Версия 03. Дата релиза 12.02.2023 */
/* Программа-калькулятор */

#include <stdio.h>

int show_menu (void);
void get_nums (int * firstNum, int * secondNum);
void put_menu (int * firstNum, int * secondNum, int * result);
void case_minus (int * firstNum, int * secondNum, int * result);
void case_plus (int * firstNum, int * secondNum, int * result);
void case_multiple (int * firstNum, int * secondNum, int * result);
void case_div (int * firstNum, int * secondNum, int *result);
void put_result (int result);

int firstNum;
int secondNum;
int result;

int main()
{
    	printf("Это программа-калькулятор целых чисел.\n");
    	put_menu(&firstNum, &secondNum, &result);
    	put_result(result);

    	return 0;
}

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

void put_menu (int * firstNum, int * secondNum, int * result)
{
    	get_nums (firstNum, secondNum);

    	int answer = 5;
    
    	answer = show_menu();
    
    	switch(answer)
    	{
    		case 1:
        		case_minus(firstNum, secondNum, result);
			break;
		case 2:
			case_plus(firstNum, secondNum, result);
			break;
		case 3:
			case_multiple(firstNum, secondNum, result);
			break;
		case 4:
			case_div(firstNum, secondNum, result);
			break;
	}
    
    	/*
    	while(answer != 0)
	{
		answer = show_menu();

		switch(answer)
    		{	
			case 1:
        			case_minus(firstNum, secondNum, result);
				break;
			case 2:
				case_plus(firstNum, secondNum, result);
				break;
			case 3:
				case_multiple(firstNum, secondNum, result);
				break;
			case 4:
				case_div(firstNum, secondNum, result);
				break;
			default:
				printf("Неправильный ввод! Попробуйте еще раз."); // Получается бесконечный цикл
        			break;
		}
    	}*/
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


