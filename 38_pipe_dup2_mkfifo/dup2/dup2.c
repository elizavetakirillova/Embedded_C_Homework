/* Использование канала для соединения двух команд 
Требуется передать две команды ./dup2 ls wc */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

int main(int args, char* argv[])
{
    int pfd[2]; // Файловые дескрипторы канала
    
    if (pipe(pfd) == -1) // Создаем канал
        perror("pipe");
    
    switch(fork())
    {
        case -1:
            perror("fork");
        case 0: // Первый потомок выполняет первую команду, записывая результат в канал
            if (close(pfd[0]) == -1) // Считывающий конец не используется
                perror("close 1");
                
            // Дублируем стандартный вывод на записывающем конце канала,
            // закрываем лишний дескриптор
            
            if (pfd[1] != STDOUT_FILENO) // Если не ошибка
            {
                if (dup2(pfd[1], STDOUT_FILENO) == -1)
                    perror("dup2 1");
                if (close(pfd[1]) == -1)
                    perror("close 2");
            }
            
            execlp(argv[1], argv[1], (char *) NULL); // Записывает в канал
            perror("execlp ls");
        
        default: // Родитель выходит из этого блока, чтобы создать след-го потомка
            break;
    }
    
    switch (fork())
    {
        case -1:
            perror("fork 2");
        case 0: // Второй потомок выполняет вторую команду, считывая ввод из канала
            if (close(pfd[1]) == -1) // Записывающий конец не используется
                perror("close 3");
            
            // Дублируем стандартный вывод на записывающем конце канала,
            // закрываем лишний дескриптор  
            
            if (pfd[0] != STDIN_FILENO)
            {
                if (dup2(pfd[0], STDIN_FILENO) == -1)
                    perror("dup2 2");
                if (close(pfd[0]) == -1)
                    perror("close 4");
            }
            
            execlp(argv[2], argv[2], (char *) NULL); // Читает из канала 
            perror("execlp wc ");
            
        default:
            break;         
    }
    
    // Родитель закрывает лишние деск-ры канала и ждет завершения дочерних проц-в
    
    if (close(pfd[0]) == -1)
        perror("close 5");
    if (close(pfd[1]) == -1)
        perror("close 6");
    if (wait(NULL) == -1)
        perror("wait 1");
    if (wait(NULL) == -1)
        perror("wait 2");
    
    exit(EXIT_SUCCESS);
}
