/* Программа представляет собой базу данных абонентов телефонной книги,
однованную на однонаправленном связном списке с возможностью
внесения данных пользователем.

Елизавета Кириллова. Версия 01. Дата релиза 23.01.2023 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
// #include <conio.h>
#include <malloc.h>
#include <termios.h>

#define CLS system("cls"); // Очистить экран
#define FFLUSH while(getchar() != '\n') // Опустошить поток ввода-вывода

/* Будем хранить данные абонентов */
struct telephone_list
{
    //char *firstName;
    //char *secondName;
    long int phoneNumb;
    struct telephone_list * next;
    // struct telephone_list * prev;
};

struct telephone_list * first;
struct telephone_list * newNode;
struct telephone_list * current;

int showMenu(void); /* Показать меню */
void addNode(void); /* Добавить новый узел в конец списка */
void removeNode(void); /* Удалить узел */
void showList(void); /* Показать список */
struct telephone_list * memAlloc(void); /* Выделить память под узел */

/* Все, чем будет заниматься ф-ция main - вызов других функций */
int main()
{
    int choice = 5; /* Выбор пользователя */
    first = NULL; /* Изначально список пуст */

    while(choice != 0) /* Пока не выбран Q - обрабатывать список */
    {
        choice = showMenu();

        switch(choice)
        {
            case 1: // С буквами не получается
                //CLS;
                showList();
                break;
            case 2:
                //CLS;
                addNode();
                break;
            case 3:
                //CLS;
                removeNode();
                break;
            case 4:
                puts("Exit...");
                system("pause");
                break;
            default:
                //CLS;
                puts("Incorrect input.");
                break;
        }
    }
}

/* Вывод меню и получение ввода пользователя */
int showMenu(void)
{
    int ch = '\0';
    printf("1 - Show list, 2 - Add node, 3 - Remove node, 4 - Exit\n");
    scanf(" %d", &ch); /* Для интерактивности */
    FFLUSH;
    return(ch);
}

/* Выделение памяти под узел */
struct telephone_list * memAlloc(void)
{
    struct telephone_list * temp;
    temp = (struct telephone_list *)malloc(sizeof(struct telephone_list));
    if(temp == NULL)
    {
        puts("Error of allocation! Exit...");
        system("pause");
        exit(1);
    }
    return temp;
};

/* Добавляем узел в конец списка */
void addNode(void)
{
    if(first == NULL) /* Список пуст */
    {
        first = memAlloc(); /* Создаем первый элемент (выделяем память) */
        current = first;
    }
    else /* Если список не пустой - есть >= 1 узла */
    {
        current = first; /* Сделать первый элемент текущим */
        while(current->next != NULL) /* Найти конец списка */
        {
            current = current->next; /* Делаем следующий узел текущим */
            newNode = memAlloc(); /* Создаем новый узел */
            current->next = newNode; /* Новый узел теперь последний в списке */
            current = newNode; /* Новый узел теперь текущий */
        }
    }
    current->next = NULL; /* "Закрываем" список, сколько бы там ни было узлов */

    /* Получаем значения от пользователя */

    //printf("Enter имя: ");
    //scanf(" %s", &current->firstName);
    //FFLUSH;
    /*printf("Enter order number: ");
    scanf("%d", &current->phoneNumb);
    FFLUSH;*/

    printf("Enter order number: ");
    scanf("%d", &current->phoneNumb);
    FFLUSH;

    /*printf("Enter имя: ");
    scanf(" %s", &current->firstName);
    FFLUSH;

    printf("Enter фамилию: ");
    scanf(" %s", &current->firstName);
    FFLUSH;

    /*current->phoneNumb = -1;
    while(current->phoneNumb < 0 || current->phoneNumb > 100000000000)
    {
        printf("Enter order number: ");
        scanf("%d", &current->phoneNumb);
        FFLUSH;
    }

    while(current->firstName < 0 || current->firstName > 100000000000)
    {
        printf("Enter имя: ");
        scanf(" %s", &current->firstName);
        FFLUSH;
    }*/
}

/* Вывод всего списка на экран */
void showList(void)
{
    int count = 1; /* Порядковый номер узла */

    if(first == NULL) /* Список пуст */
    {
        puts("List is empty");
        return;
    }
    puts("List:");

    /* Проходим последовательно по всему списку */
    current = first;
    while(current != NULL)
    {
        printf("Node %d: number %d",
                count, /*current->firstName,*/ current->phoneNumb); /* Вывод содержания узла на экран *abonent %s */
        /* Перейти к следующему узлу */
        current = current->next; /* Перейти к следующему узлу */
        count++;
    }
}

/* Удаление узла из списка */
void removeNode(void)
{
    struct lelephone_list * prev; /* Предыдущий узел, тот, что находится за удаляемым узлом */
    int rem = 0; /* Порядковый номер удаляемого узла */
    int num; /* Счетчик для цикла - для прохода по списку */

    if(first == NULL) /* Если список пуст */
    {
        puts("Nothing to remove! List is epty.");
        return;
    }
    showList(); /* Покажем список, чтобы пользователю было удобно выбирать элемент */

    while(rem < 1 || rem > 100)
    {
        puts("Choose number of node for remove:");
        scanf("%d", &rem);
        FFLUSH;
    }

    /* Здесь самое важное */
    num = 1;
    current = first; /* Начинаем проход списка с первого узла */
    prev = NULL; /* У первого узла нет предыдущего */
    while(num != rem) /* Идем до узла номер rem */
    {
        prev = current; /* Прежде чем перейти к следующему узлу */
        current = current->next; /* Следующий узел */
        num++;
        if(current == NULL) /* Если дошли до конца списка */
        {
            puts("Node not found.");
            return;
        }
    }
    if(prev == NULL) /* Если удаляется первый узел */
        first = current->next; /* Сделать второй узел первым */
    else
        prev = current->next; /* Предыдущий указывает на следующий
        в обход текущего */
    printf("Node #%d is removed\n", rem);
    free(current);
}



