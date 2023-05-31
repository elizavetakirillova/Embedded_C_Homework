#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>


#define FTOKSEMPATH "/tmp/lab6sem" /*Путь к файлу, передаваемому ftok для набора семафоров */
#define FTOKSHMPATH "/tmp/lab6shm" /*Путь к файлу, передаваемому ftok для разделяемого сегмента */
#define FTOKID 1                   /*Идентификатор, передаваемый ftok */

#define NUMSEMS 2                  /* Число семафоров в наборе */
#define SIZEOFSHMSEG 512           /* Размер сегмента разделяемой памяти */

#define NUMMSG 12                  /* Число принимаемых сообщений */


int main(void)
{
             int semid, shmid;
             key_t semkey, shmkey;
             void *shm_address;
             struct sembuf operations[2];
             struct shmid_ds shmid_struct;

             /*Создание файлов для ftok*/
             fclose(fopen(FTOKSEMPATH,"w"));
             fclose(fopen(FTOKSHMPATH,"w"));

             /*Создание IPC-ключей*/
             semkey = ftok(FTOKSEMPATH,FTOKID);
             if ( semkey == (key_t)-1 )
             {
                 printf("Сервер: ошибка при выполнении %s\n","semkey = ftok(FTOKSEMPATH,FTOKID);");
                 return -1;
             }
             shmkey = ftok(FTOKSHMPATH,FTOKID);
             if ( shmkey == (key_t)-1 )
             {
                 printf("Сервер: ошибка при выполнении %s\n","shmkey = ftok(FTOKSHMPATH,FTOKID);");
                 return -1;
             }

             /*Создание набора семафоров с помощью IPC-ключей*/
             semid = semget( semkey, NUMSEMS, 0666 | IPC_CREAT | IPC_EXCL);
             if ( semid == -1 )
             {
                 printf("Сервер: ошибка при выполнении %s\n","semid = semget( semkey, NUMSEMS, 0666 | IPC_CREAT | IPC_EXCL);");
                 return -1;
             }

             /* Произведём 2 семафора:
                  Истина в 0 семафоре - область разделяемой памяти используется.
                  Истина в 1 семафоре - область разделяемой памяти изменена клиентом.*/

             /*Инициализация семафоров*/
             if(semctl( semid, 0, SETVAL, 0) == -1)
             {
                 printf("Сервер: ошибка инициализации 0 семафора: %s\n","semctl( semid, 0, SETVAL, 0) == -1.");
                 return -1;
             }

             if(semctl( semid, 1, SETVAL, 0) == -1)
             {
                 printf("Сервер: ошибка инициализации 1 семафора: %s\n","semctl( semid, 1, SETVAL, 0) == -1.");
                 return -1;
             }

             /*Создание сегмента разделяемой памяти*/
             shmid = shmget(shmkey, SIZEOFSHMSEG, 0666 | IPC_CREAT | IPC_EXCL);
             if (shmid == -1)
             {
                 printf("Сервер: ошибка при выполнении %s\n","shmid = shmget(shmkey, SIZEOFSHMSEG, 0666 | IPC_CREAT | IPC_EXCL);");
                 return -1;
             }

             /*Прикрепление сегмента разделяемой памяти, получение адреса*/
             shm_address = shmat(shmid, NULL, 0);
             if ( shm_address==NULL )
             {
                 printf("Сервер: ошибка при выполнении %s\n","shm_address = shmat(shmid, NULL, 0);");
                 return -1;
             }
             printf("Сервер готов принимать сообщения от клиентов. Сервер настроен на прием %d сообщений.\n\n", NUMMSG);

             /*Цикл обработки сообщений. Выполняется NUMMSG раз*/
             for (int i = 0; i < NUMMSG; i++)
             {
               /* Сервер ожидает появления Истину на втором семафоре (сегмент разделяемой памяти изменен клиентом), затем выставляет 1 на первом семафоре (сегмент занят) */
                 operations[0].sem_num = 1;
                 operations[0].sem_op = -1;
                 operations[0].sem_flg = 0;

                 operations[1].sem_num = 0;
                 operations[1].sem_op =  1;
                 operations[1].sem_flg = IPC_NOWAIT;

                 if (semop( semid, operations, 2 ) == -1)
                 {
                   printf("Сервер: ошибка при выполнении %s\n","semop( semid, operations, 2 ) == -1.");
                 }

                 /*Обработать сообщение, полученное от клиента*/
                 printf(">> #%s#\n", (char *) shm_address);

                 /*Установить первый семафор в 0 (сегмент свободен)*/
                 operations[0].sem_num = 0;
                 operations[0].sem_op  = -1;
                 operations[0].sem_flg = IPC_NOWAIT;

                 if (semop( semid, operations, 1 ) == -1)
                 {
                   printf("Сервер: ошибка при выполнении %s\n","semop( semid, operations, 1 ) == -1.");
                   return -1;
                 }

             } /* Конец цикла обработки сообщений. */

             if (semctl( semid, 1, IPC_RMID ) == -1)
             {
               printf("Сервер: ошибка освобождения семафоров: %s\n","semctl( semid, 1, IPC_RMID ) == -1.");
               return -1;
             }

             if (shmdt( shm_address ) == -1)
             {
               printf("Сервер: ошибка открепления сегмента разделяемой памяти: %s\n","shmdt(shm_address) == -1.");
               return -1;
             }

             if (shmctl( shmid, IPC_RMID, &shmid_struct ) == -1)
             {
               printf("Сервер: ошибка освобождения сегмента разделяемой памяти: %s\n","shmctl(shmid, IPC_RMID, &shmid_struct) == -1.");
               return -1;
             }

             /*Удаление файлов для ftok*/
             unlink(FTOKSHMPATH);
             unlink(FTOKSEMPATH);
             return 0;
}
