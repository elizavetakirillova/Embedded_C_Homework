/*
 * Клиент: записывает строки в сегмент разделяемой памяти
 *
 * Пример работы клиента для записи информации в разделяемую память
 *
 * Дополнительная информация: https://www.softprayog.in/programming/interprocess-communication-using-system-v-shared-memory-in-linux
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
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

    // инициализация мьютекса управления потоками
    // производит взаимное исключение нахожддения потоков в критической секции
    if ((s_key = ftok (SEM_MUTEX_KEY, PROJECT_ID)) == -1) // генерация уникального ключа из файлового дескриптора
        error ("ftok");
    if ((mutex_sem = semget (s_key, 1, 0)) == -1)
        error ("semget");

    // получение доступа к разделяемой памяти
    if ((s_key = ftok (SHARED_MEMORY_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((shm_id = shmget (s_key, sizeof (struct shared_memory), 0)) == -1)
        error ("shmget");
    if ((shared_mem_ptr = (struct shared_memory *) shmat (shm_id, NULL, 0))
         == (struct shared_memory *) -1)
        error ("shmat");

    // семафор для счета количества свободных буферов для записи сообщений
    if ((s_key = ftok (SEM_BUFFER_COUNT_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((buffer_count_sem = semget (s_key, 1, 0)) == -1)
        error ("semget");

    // семафор для счета количества сообщений необходимых для печати
    if ((s_key = ftok (SEM_SPOOL_SIGNAL_KEY, PROJECT_ID)) == -1)
        error ("ftok");
    if ((spool_signal_sem = semget (s_key, 1, 0)) == -1)
        error ("semget");

    // структура для управления состоянием семафора
    struct sembuf asem [1];
    asem [0].sem_num = 0;
    asem [0].sem_op = 0; // флаг операции над счетчиком семафора -1 вычитание из счетчика, 1 прибавление к счетчику
    asem [0].sem_flg = 0;

    // буфер для сохранения пользовательского ввода
    char buf [200];

    printf ("Please type a message: ");

    // основной цикл работы клиента
    while (fgets (buf, 198, stdin)) {

        // убираем символы переноса строки из буфера сообщения
        int length = strlen (buf);
        if (buf [length - 1] == '\n')
           buf [length - 1] = '\0';

        // блокируем buffer_count_sem записи сообщения клиентом в буфер
        // если здесь будет 0, то ядро защититься, так как значение семафора не может быть меньше 0
        // процесс заблокируется до тех пор, пока сообщение не будет обработано спуллером
        // и спуллер не вызовет разблокировки данного семафора, увеличив счетчик на 1
        // (разблокировку семафора buffer_count_sem нужно произвести после печати и изменения индекса печати в спуллере)
        asem [0].sem_op = -1;
        if (semop (buffer_count_sem, asem, 1) == -1)
	       error ("semop: buffer_count_sem");

        // блокируем mutex_sem, чтобы процессы клиентов не пытались одновременно писать в один и тот же буфер
        // пока mutex_sem заблокирован, изменения в буфере может производить только один клиент
        asem [0].sem_op = -1;
        if (semop (mutex_sem, asem, 1) == -1)
	       error ("semop: mutex_sem");

	      // выполняем операцию записи сообщения в буфер по адресу buffer_index
        sprintf(shared_mem_ptr -> buf [shared_mem_ptr -> buffer_index], "(%d): %s\n", getpid (), buf);

        // изменяем индекс буфера сообщения
        (shared_mem_ptr -> buffer_index)++;
        if (shared_mem_ptr -> buffer_index == MAX_BUFFERS)
          shared_mem_ptr -> buffer_index = 0; // ограничиваем индекс буфера

        // разблокируем mutex_sem для работы остальных клиентов
        asem [0].sem_op = 1;
        if (semop (mutex_sem, asem, 1) == -1)
	       error ("semop: mutex_sem");

	      // разблокируем семафор spool_signal_sem для печати и обработки сообщения спуллером
        asem [0].sem_op = 1;
        if (semop (spool_signal_sem, asem, 1) == -1)
	       error ("semop: spool_signal_sem");

        // печатаем инвайт для ввода нового сообщения
        printf ("Please type a message: ");
    }

    // отсоединяем сегмент shared_memory от адресного пространства данного процесса
    if (shmdt ((void *) shared_mem_ptr) == -1)
        error ("shmdt");

    // выходим из программы
    exit (0);
}

// печать ошибки и выход из программ
void error (char *msg)
{
    perror (msg);
    exit (1);
}
