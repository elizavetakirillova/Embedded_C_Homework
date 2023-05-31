/* Кириллова Е.С.
Программа демонстрирует применение побитовых операций */

#include <stdio.h>

int main()
{
    int a = 0xDDCCBBAA;
    char *ptr = &a;

    for(int i = 0; i < 4; i++)
    {
        printf("%d byte of a = %x\n", i, *ptr);
        ptr = ptr+1;
    }

    ptr = ptr + 2;
    *ptr = 2;

    printf("third byte of a = %d\n", *ptr);

    return 0;
}
