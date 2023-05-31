/* Кириллова Е.С.
Программа заполняет верхний треугольник матрицы 0, а нижний 1 */

#include <stdio.h>
#define ROWS 4
#define COLUMNS 3

int main()
{
    int i; // Счетчик строк
    int d; // Счетчик колонок
    int c; // Лимит колонок

    int myArr[ROWS][COLUMNS];

    for(i = 0, c = COLUMNS; i < ROWS; i++, c--)
    {
        for(d = 0; d < c; d++)
        {
            myArr[i][d] = 0;
            printf("%d", myArr[i][d]);
        }

        for(d = c; d < COLUMNS; d++)
        {
            myArr[i][d] = 1;
            printf("%d", myArr[i][d]);
        }

        printf("\n");
    }

    return 0;
}
