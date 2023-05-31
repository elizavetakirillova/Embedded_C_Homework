/* Программа-калькулятор с динамической библиотекой,
подгружаемой через функции.
Елизавета Кириллова. Версия 02. Дата релиза 12.02.2023 */

#include "my_func.h"
#include <stdio.h>
#include <string.h>
#include <dlfcn.h> // Для использования dlopen, dlclose и др.

int firstNum;
int secondNum;
int result;

int main(int argc, char* argv[])
{
    void* handle;
    void (*funcp)(void);
    const char* err;
    
    if (argc != 2 || strcmp(argv[1], "--help") == 0)
    	perror("error args");
    
    /* Загружаем разделяемую библиотеку и получаем дескриптор для ее
    дальнейшего использования */
    
    handle = dlopen("./libmyname.so", RTLD_LAZY);
    if (handle == NULL)
    	perror("dlopen err");
    
    /* Ищем в библиотеке с именем, заданным в argv[1] */
    
    (void) dlerror(); /* Очищаем ошибки с помощью dlerror() */
    
    *(void**) (&funcp) = dlsym(handle, argv[1]);
    err = dlerror();
    if (err != NULL)
    	perror("dlsymerr");
    
    /* Если адрес, возвращенный dlsym(), не равен NULL, пытаемся вызвать 
    его значение как функцию, которая не принимает аргументов */
    
    if (funcp == NULL)
    	printf("%s is NULL\n", argv[1]);
    else
    	(*funcp)();
    
    dlclose(handle);
    
    return 0;
    
    /*printf("Это программа-калькулятор целых чисел.\n");
    	
    void (* libget_nums_ptr)(int, int); // Создаем указатель на функцию в библ-ке
    	
    handle = dlopen(libget_nums.so, RTLD_LAZY); // Открываем библ-ку м этой ф-цией
    	if (!handle) {
    		fprintf(stderr, "%s\n", dlerror());
    		exit(EXIT_FAILURE);
    	}
	dlerror(); // Очистка всех результатов ошибок
	libget_nums_ptr = dlsym(get_nums_ptr, "cos"); // Выдергиваем ф-ции из этой библ-ки
    	error = dlerror();
    	
    	if(error != NULL)
    	{
    		fprintf(stderr, "%s\n", error);
    		exit(EXIT_FAILURE);
    	}    	
    	
    	dlclose(handle);
    	exit(EXIT_SUCCESS);
    	
    	
    	
    	// get_nums (& firstNum, & secondNum);

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

    	return 0;*/
}



