/*
 * Спуллер: печатает строки из сегмента разделяемой памяти, записанные клиентами
 *
 * Пример создания и обработки информации из разделяемой памяти
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

// Основные константы
#define MAX_BUFFERS 10 // максимальное количество буферов сообщений в разделяемой памяти
#define SHARED_MEMORY_KEY "/tmp/shared_memory_key" // путь к временному файлу-дескриптору разделяемой памяти
#define SEM_MUTEX_KEY "/tmp/sem-mutex-key" // путь к временному файлу-дескриптору мьютекса для управления потоками (при многопоточности)
#define SEM_BUFFER_COUNT_KEY "/tmp/sem-buffer-count-key" // путь к временному файлу-дескриптору счетчика буфера (для определения текущего положения чтения в буфере разделяемой памяти)
#define SEM_SPOOL_SIGNAL_KEY "/tmp/sem-spool-signal-key" // путь к временному файл-дескриптору счетчика управления спуллером (для обработки нового сообщения)
#define PROJECT_ID 'K' // уникальный ключ приложения

// описание структуры разделяемой памяти
struct shared_memory {
    char buf [MAX_BUFFERS] [256]; // буфер сообщений
    int buffer_index; // индекс в буфере сообщений
    int buffer_print_index; // индекс в буфере сообщений для печати сообщения
};

void error (char *msg);

int main (int argc, char **argv)
{
  key_t s_key; // ключ доступа к структурам IPC

  // структура семафора
  union semun
  {
      int val; // значение семафора
      struct semid_ds *buf;
      ushort array [1];
  } sem_attr;

  // информация по структуре семафора
  // struct semid_ds {
  //         struct ipc_perm sem_perm;       /* права доступа */
  //         time_t          sem_otime;      /* время последней операции semop */
  //         time_t          sem_ctime;      /* время последнего изменения */
  //         struct sem      *sem_base;      /* указатель к первому семафору в массиве */
  //         struct wait_queue *eventn;
  //         struct wait_queue *eventz;
  //         struct sem_undo  *undo;         /* число запросов */
  //         ushort          sem_nsems;      /* число семафоров */
  // };

  // информация по правам доступа к семафору
  // struct ipc_perm {
  //   ushort	 uid;	     /* uid владельца */
  //   ushort	 gid;	     /* gid группы */
  //   ushort	 cuid;	   /* сuid создателя */
  //   ushort	 cgid;	   /* cgid создателя-группы */
  //   ushort	 mode;	   /* режим доступа */
  //   ushort	 seq;	     /* номер последовательности используемого слота */
  //   key_t	 key;	     /* ключ */
  //};

  int shm_id; // идентификатор разделяемой памяти
  struct shared_memory *shared_mem_ptr; // указатель на разделяемой памяти
  int mutex_sem, buffer_count_sem, spool_signal_sem; // объявление семафороф (для многопоточности, счетчика буфера, управления спуллером)

    printf ("spooler: hello world\n");
    //  инициализация мьютекса для управления потоками
    if ((s_key = ftok (SEM_MUTEX_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((mutex_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // установ начального значения мьютекса
    sem_attr.val = 0; // заблокирован до завершения процесса инициализации
    if (semctl (mutex_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // создание и получение доступа к сегменту разделяемой памяти
    if ((s_key = ftok (SHARED_MEMORY_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((shm_id = shmget (s_key, sizeof (struct shared_memory),
         0660 | IPC_CREAT)) == -1)
        error ("shmget");
    if ((shared_mem_ptr = (struct shared_memory *) shmat (shm_id, NULL, 0))
         == (struct shared_memory *) -1)
        error ("shmat");
    // инициализация индексов буфера разделяемой памяти
    shared_mem_ptr -> buffer_index = shared_mem_ptr -> buffer_print_index = 0;

    // инициализация семафора для счета количества доступных буферов для записи сообщений
    if ((s_key = ftok (SEM_BUFFER_COUNT_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((buffer_count_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // установ начального значения семафора
    sem_attr.val = MAX_BUFFERS; // доступно MAX_BUFFERS для запси сообщений
    if (semctl (buffer_count_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // инициализация семафора для счета количества строк для печати
    if ((s_key = ftok (SEM_SPOOL_SIGNAL_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((spool_signal_sem = semget (s_key, 1, 0660 | IPC_CREAT)) == -1)
        error ("semget");
    // установ начального значения семафора
    sem_attr.val = 0; // 0 сообщений доступно для печати (до того как их введут пользователи клиентов)
    if (semctl (spool_signal_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // инициализация завершена
    // разблокируем мьютекс доступа к разделяемой памяти для клиентов
    sem_attr.val = 1;
    if (semctl (mutex_sem, 0, SETVAL, sem_attr) == -1)
        error ("semctl SETVAL");

    // структура для управления состоянием семафора
    struct sembuf asem [1];
    asem [0].sem_num = 0;
    asem [0].sem_op = 0; // флаг операции над счетчиком семафора -1 вычитание из счетчика, 1 прибавление к счетчику
    asem [0].sem_flg = 0;

    // основной цикл работы спуллера
    while (1) {

        // определяем есть ли еще строки для печати по состоянию spool_signal_sem
        // если значение семафора равно нулю, то либо сообщений не было, либо они все обработаны в данном цикле
        // соответственно ядро защититься, так как значение семафора не может быть меньше нуля
        // процесс будет заблокирован до тех пор, пока клиент добавит новое сообщение в буфер и не разблокирует данный семафор
        // увеличив счетчик семафора спуллера на единицу (это нужно сделать после добавления сообщения в буфер)
        asem [0].sem_op = -1;
        if (semop (spool_signal_sem, asem, 1) == -1)
	       perror ("semop: spool_signal_sem");

        // печатаем сообщение
        printf ("%s", shared_mem_ptr -> buf [shared_mem_ptr -> buffer_print_index]);

        // так существует только один процесс спуллера, нет необходимости
        // в использовании мьютекса mutex_sem, но при необходимости его можно добавить

        // вход в критическую секцию
        (shared_mem_ptr -> buffer_print_index)++; // увеличиваем индекс буфера для печати сообщения на единицу
        if (shared_mem_ptr -> buffer_print_index == MAX_BUFFERS)
           shared_mem_ptr -> buffer_print_index = 0; // ограничиваем диапазон значений индекса буфера

        // содержимое буффера было считано для печати, теперь освобождаем буфер для дальнейшей записи сообщений
        // разблокируем семафор buffer_count_sem для дальнейшего ввода сообщений клиентами
        asem [0].sem_op = 1;
        if (semop (buffer_count_sem, asem, 1) == -1)
	       perror ("semop: buffer_count_sem");
    }
}

// печетаем сообщение и выходим из программы с кодом ошибки
void error (char *msg)
{
    perror (msg);
    exit(1);
}
