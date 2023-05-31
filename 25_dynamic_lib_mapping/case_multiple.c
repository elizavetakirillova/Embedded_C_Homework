#include <stdio.h>
#include <string.h>
#include <dlfcn.h> 

void case_multiple (int * firstNum, int * secondNum, int * result)
{
    	* result = (* firstNum) * (* secondNum);
}
