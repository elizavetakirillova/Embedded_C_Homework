#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>

#define FTOKSEMPATH "/tmp/lab6sem" /*Путь к файлу, передаваемому ftok для набора семафоров */
#define FTOKSHMPATH "/tmp/lab6shm" /*Путь к файлу, передаваемому ftok для разделяемого сегмента */
#define FTOKID 1                   /*Идентификатор, передаваемый ftok */

#define NUMSEMS 2                  /* Число семафоров в наборе */
#define SIZEOFSHMSEG 512           /* Размер сегмента разделяемой памяти */

#define NUMMSG 3                   /* Число передаваемых сообщений */

int main(void)
{
  struct sembuf operations[3];
  void         *shm_address;
  int semid, shmid;
  key_t semkey, shmkey;

  /*Создание IPC-ключей*/
  semkey = ftok(FTOKSEMPATH,FTOKID);
  if ( semkey == (key_t)-1 )
  {
    printf("Клиент: ошибка при выполнении %s\n","semkey = ftok(FTOKSEMPATH,FTOKID);");
    return -1;
  }
  shmkey = ftok(FTOKSHMPATH,FTOKID);
  if ( shmkey == (key_t)-1 )
  {
    printf("Клиент: ошибка при выполнении %s\n","shmkey = ftok(FTOKSHMPATH,FTOKID);");
    return -1;
  }

  /*Получение набора семафоров с помощью IPC-ключей*/
  semid = semget( semkey, NUMSEMS, 0666);
  if ( semid == -1 )
  {
    printf("Клиент: ошибка при выполнении %s\n","semid = semget( semkey, NUMSEMS, 0666);");
    return -1;
  }

  /*Получение сегмента разделяемой памяти*/
  shmid = shmget(shmkey, SIZEOFSHMSEG, 0666);
  if (shmid == -1)
  {
    printf("Клиент: ошибка при выполнении %s\n","shmid = shmget(shmkey, SIZEOFSHMSEG, 0666);");
    return -1;
  }

  /*Прикрепление сегмента разделяемой памяти, получение адреса*/
  shm_address = shmat(shmid, NULL, 0);
  if ( shm_address==NULL )
  {
    printf("Клиент: ошибка при выполнении %s\n","shm_address = shmat(shmid, NULL, 0);");
    return -1;
  }

  /*Цикл отправки сообщений. Выполняется NUMMSG раз*/
  for (int i = 0; i < NUMMSG; i++)
  {
    /* Клиент ожидает появления Лжи на 0 семафоре (сегмент разделяемой памяти свободен) и Лжи на 1 семафоре (сегмент обработан сервером) затем выставляет Истину на 0 семафоре (сегмент занят) */

    operations[0].sem_num = 0;
    operations[0].sem_op =  0;
    operations[0].sem_flg = 0;

    operations[1].sem_num = 1;
    operations[1].sem_op =  0;
    operations[1].sem_flg = 0;

    operations[2].sem_num = 0;
    operations[2].sem_op =  1;
    operations[2].sem_flg = 0;

    if (semop( semid, operations, 3 ) == -1)
    {
      printf("Клиент: ошибка при выполнении %s\n","semop( semid, operations, 2 ) == -1.");
      return -1;
    }

    snprintf( (char *) shm_address, SIZEOFSHMSEG, "(Само сообщение) pid=%d", getpid() );
    usleep(200);
    /* Установить 0 семафор в 0 (сегмент свободен)
       Установить 1 семафор в 1 (сегмент изменен).*/
    operations[0].sem_num = 0;
    operations[0].sem_op =  -1;
    operations[0].sem_flg = 0;

    operations[1].sem_num = 1;
    operations[1].sem_op =  1;
    operations[1].sem_flg = 0;
    if (semop( semid, operations, 2 ) == -1)
    {
      printf("Клиент: ошибка при изменении семафоров: %s\n","semop( semid, operations, 2 ) == -1.");
      return -1;
    }
  }  /* Конец цикла отправки сообщений */

  /*Открепление сегмента разделяемой памяти.*/
  if (shmdt(shm_address) == -1)
  {
    printf("Клиент: ошибка при откреплении сегмента разделяемой памяти: %s\n","shmdt(shm_address) == -1.");
    return -1;
  }

  return 0;
}
