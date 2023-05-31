/* Программа-калькулятор с динамической библиотекой
Елизавета Кириллова. Версия 02. Дата релиза 12.02.2023 */

#include "my_func.h"
#include <stdio.h>

int firstNum;
int secondNum;
int result;

int main()
{
    	printf("Это программа-калькулятор целых чисел.\n");
    	
    	get_nums (& firstNum, & secondNum);

    	int answer = 5;
    
    	answer = show_menu();
    
    	switch(answer)
    	{
    		case 1:
        		case_minus(& firstNum, & secondNum, & result);
			break;
		case 2:
			case_plus(& firstNum, & secondNum, & result);
			break;
		case 3:
			case_multiple(& firstNum, & secondNum, & result);
			break;
		case 4:
			case_div(& firstNum, & secondNum, & result);
			break;
	}

    	put_result(result);

    	return 0;
}



