/* Кириллова Е.С.
Вывести заданный массив размерм N в обратном порядке */

#include <stdio.h>
#define LIMIT 5

int main()
{
    int i; // Счетчик элементов массива

    int myArr[LIMIT] = {1, 2, 3, 4, 5};

    for(i = LIMIT-1; i >= 0; i--)
    {
        printf("%d", myArr[i]);
    }

    return 0;
}
