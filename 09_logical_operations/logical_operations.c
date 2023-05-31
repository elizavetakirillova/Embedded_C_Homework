/* Кириллова Е.С.
Программа демонстрирует применение побитовых операций */

#include <stdio.h>

int main()
{
    int a = 0xDDCCBBAA;
    unsigned char c;

    c = (a & 0x00FF0000); // Не работает
    printf("Third byte of a = %x\n", c);

    c = ((a>>16) & 0xFF);
    printf("Third byte of a = %x\n", c);

    c = ((a & 0x00FF0000)>>16);
    printf("Third byte of a = %x\n", c);

    c =((a<<8)>>24);
    printf("Third byte of a = %x\n", c);
}
