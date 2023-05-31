/* Телефонный справочник на основе статической структуры с возможностью заполнения пользователем.

Вначале требуется вывод текстового меню на основе switch-кейсов:
1 добавить абонента
2 поиск абонента (только по одному из полей)
3 удаление абонента
4 вывод списка
5 выход из программы

Далее программа выполняет действия, выбранные пользователем и выводит меню снова.
 */
#include <stdio.h>
#define SIZE 30
#define MAXSIZE 20

struct list
{
    char firstName[SIZE];
    char secondName[SIZE];
    int phoneNumb;
};

int count = 0; // Глобальный счетчик

//char * s_gets(char * st, int n);
void put_menu(struct list * ptr, int count);
void add_element(struct list * ptr, int count);
void put_list(struct list * ptr, int index, int count);
void find_element(struct list * ptr, int count);
void delete_element(struct list * ptr, int count);

int main()
{
    struct list myList[MAXSIZE];
    //struct list *ptr;
    //ptr = &myList;

    put_menu(myList, count);

    return 0;
}

void put_menu(struct list * ptr, int count)
{
    int i = 0;

    printf("Выберите один из пунктов меню: \n");
    printf("1. Добавить элемент\n");
    printf("2. Поиск абонента\n");
    printf("3. Удаление абонента\n");
    printf("4. Вывод списка\n");
    printf("5. Выход из программы\n\n");
    printf("Введите номер выбранного пункта: ");
    scanf("%d", &i);

    switch(i)
    {
        case 1:
            add_element(ptr, count);
            break;
        case 2:
            find_element(ptr, count);
            break;
        case 3:
            delete_element(ptr, count);
            break;
        case 4:
            put_list(ptr, 0, count);
            break;
        case 5:
            break;
        default: // добавить сценарий для возможности повторного вывода меню
            printf("Пожалуйста, введите корректный ответ.\n");
            break;
    }

    //put_menu(ptr, count);
}

void add_element(struct list * ptr, int count)
{
    printf("Введите имя абонента № %d: ", count+1);
    scanf("%s", ptr[count].firstName); // заменить scanf на s_gets

    printf("Теперь введите фамилию абонента № %d: ", count+1);
    scanf("%s", ptr[count].secondName);

    printf("Теперь введите номер абонента № %d в формате 89999999: ", count+1); // добавить возможность вводить разные форматы
    scanf("%d", &ptr[count].phoneNumb);

    printf("\n");

    count = count + 1;

    put_menu(ptr, count);
}

void find_element(struct list * ptr, int count) // добавить возможность поиска по имени, фамилии
{
    int index;
    int find;
    int count2;

    if(count == 0)
    {
        printf("\nДля поиска элемента нужен хотя бы один элемент.\n\n");
        put_menu(ptr, count);
    }

    printf("Введите номер абонента, которого вы хотите найти (в формате 89999999): ");
    scanf("%d", &find); // добавить возможнсть выхода из программы

    for(index = 0; index < count; index++)
    {
        if(ptr[index].phoneNumb == find)
        {
            printf("Ваш элемент:\n");
            count2 = index+1;
            put_list(ptr, index, count2);
        }
    }

    printf("Такого элемента нет. Попробуйте еще раз. ");
    find_element(ptr, count);
}

void delete_element(struct list * ptr, int count)
{
    int index;
    int answer;

    printf("Введите номер элемента, который вы хотите удалить: ");
    scanf("%d", &answer);

    answer--;

    if(answer == 0)
    {
        printf("Ваш список пуст.\n");
        count = 0;
        put_menu(ptr, count);
    }

    for(index = 0; index < count; index++)
    {

        if(index == answer)
        {
            ptr[index] = ptr[index++];
            count--;
            put_menu(ptr, count);
        }
    }
    printf("Такого элемента нет.");
}

void put_list(struct list * ptr, int index, int count)
{

    for(; index < count; index++)
    {
        printf("Имя абонента № %d: %s\n", index+1, ptr[index].firstName);
        printf("Фамилия абонента № %d: %s\n", index+1, ptr[index].secondName);
        printf("Номер абонента № %d: %d\n", index+1, ptr[index].phoneNumb);
        printf("\n");
    }

    put_menu(ptr, count);
}


/*
char * s_gets(char * st, int n)
{
    char * ret_val;
    char * find;

    ret_val = fgets(st, n, stdin);

    if(ret_val)
    {
        find = strchr(st, '\n');
        if (find)
            *find = '\0';
        else
            while (getchar() != '\n')
                continue;
    }
    return ret_val;
}*/
