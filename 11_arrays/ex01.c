/* Кириллова Е.С.
Программа выводит квадратную матрицу по заданному N*/

#include <stdio.h>

int main()
{
    int i; // Счетчик строк
    int d; // Счетчик колонок
    int n = 0; // Счетчик значений элементов массива
    int sizeArr // Размер массива

    printf("Введите размер массива: ");
    scanf(" %d", &sizeArr);

    int myArr[sizeArr][sizeArr];

    printf("\nВаш массив: \n");

    for(i = 0; i < sizeArr; i++)
    {
        for(d = 0; d < sizeArr; d++)
        {
            myArr[i][d] = n;
            printf("%d ", myArr[i][d]);
            n++;
        }
        printf("\n");
    }

    return 0;
}
