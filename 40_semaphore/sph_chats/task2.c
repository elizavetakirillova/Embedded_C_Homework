/* Вариант System V */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/msg.h>
#include <errno.h>
#include <signal.h>

// основные константы доступа к IPC
#define PROJECT_ID 'Z' // уникальный ключ приложения
#define SHARED_MEMORY_KEY "/tmp/shared-memory-key-1" // путь к временному файлу-дескриптору разделяемой памяти
#define SEM_MUTEX_KEY "/tmp/sem-mutex-key-1" // путь к временному файлу-дескриптору мьютекса для управления процессами

// определение параметров по умолчанию
#define MESSAGE_MAX_LENGTH 128 // длина сообщения в байтах по умолчанию
#define DEFAULT_KEY 1234 // ключ для создания очереди сообщений по умолчанию
#define DEFAULT_MODE 0 // 0 клиент, 1 сервер
#define MAX_CLIENTS 10 // максимальное количество клиентов для обслуживания сервером

// идентификатор и имя сервера
#define SERVER_ID MAX_CLIENTS + 41
#define SERVER_NAME "MAIN QUESTION ANSWER IS 42"

// временные константы (в секундах)
#define TIME_DELAY_CYCLE 1 // время задерки между циклами обработки сообщений
#define TIME_WAIT_END_JOB 15 // время ожидания завершения программы

// возможные типы сообщений
#define MESSAGE_CLIENT_ID_REQUEST 1
#define MESSAGE_CLIENT_ID_SET 2
#define MESSAGE_POST_REQUEST 3
#define MESSAGE_OUT_CHAT 4
#define MESSAGE_MAGIC_NUMBER 5

// возможные виды сообщений
#define MESSAGE_KIND_MESSAGE 0
#define MESSAGE_KIND_OUT 1
#define MESSAGE_KIND_IN 2

// возможные состояния сообщений
#define MESSAGE_NONE 0
#define MESSAGE_SENT 1
#define MESSAGE_RECEIVED 2
#define MESSAGE_ERROR - 1

// возможные состояния client_id
#define CLIENT_ID_FREE 0
#define CLIENT_ID_TAKED 1

// максимальное количество сообщений хранимое в разделяемой памяти
#define MAX_MESSAGES MAX_CLIENTS * 10

// флаги управления работой программы
static volatile int program_exit_flag = 0; // флаг обработки выхода из программы
static volatile int verbose = 0; // флаг управления выводом служебных сообщений

// структура данных сообщения
struct message_data {
  long kind; // вид сообщения
  long state; // состояние сообщения
  char writer_name[MESSAGE_MAX_LENGTH]; // имя отправителя
  long writer_id; // id отправителя
  char text[MESSAGE_MAX_LENGTH]; // текст сообщения
};

// структура буфера с сообщением
struct message_buffer {
  long type; // тип сообщения (обязательный параметр типа long, все остальные параметры произвольные)
  struct message_data data;
};

// описание структуры разделяемой памяти
struct shared_memory {
  struct message_buffer messages[MAX_MESSAGES]; // хранит сообщения
};

// структура для управления состоянием разделяемой памяти
struct shared_memory_control {
  int shm_id; // идентификатор разделяемой памяти
  struct shared_memory * shared_mem_ptr; // указатель на разделяемую память
  int mutex_sem; // объявление семафороф (мьютекс для поддержки многопоточности, семафор счетчика буфера, семафор управления сервером)
};

// структура семафора
union semun {
  int val; // значение семафора
  struct semid_ds * buf;
  ushort array[1];
};

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

// функция аварийного выхода из программы
void error(char * error_message) {
  perror(error_message); // печать сообщения об ошибке
  exit(EXIT_FAILURE);
};

// отправка сообщения в буфер shared_memory
void write_message(struct shared_memory_control shm_ctl, char * writer_name, int writer_id, int type, int kind, char * text) {

  // if (verbose) printf("[writer name, id -- %s, %d]\n", writer_name, writer_id);

  // структура для управления состоянием семафора
  struct sembuf asem[1];
  asem[0].sem_num = 0;
  asem[0].sem_op = 0; // флаг операции над счетчиком семафора -1 вычитание из счетчика, 1 прибавление к счетчику
  asem[0].sem_flg = 0;

  // if (verbose) printf("[write message lock... mutex_sem]\n");
  // блокируем write_mutex записи сообщения клиентом в буфер
  // если здесь будет 0, то ядро защититься, так как значение семафора не может быть меньше 0
  // процесс заблокируется до тех пор, пока сообщение не будет прочитано
  // и читающий процесс не вызовет разблокировки данного семафора, увеличив счетчик на 1
  // (разблокировку семафора write_mutex нужно произвести после обработки сообщения в читающем процессе)
  asem[0].sem_op = -1;
  if (semop(shm_ctl.mutex_sem, asem, 1) == -1)
    error("[semop: mutex_sem]");
  // if (verbose) printf("[write message lock... mutex_sem ok!]\n");

  // получение свободного буфера для записи сообщения
  struct message_buffer * message;
  for (int i = 0; i < MAX_MESSAGES; i++) {
    if (shm_ctl.shared_mem_ptr->messages[i].data.state == MESSAGE_NONE) {
      message = & shm_ctl.shared_mem_ptr->messages[i];
      break;
    }
  } // здесь может быть ситуация, когда в буфере нет места для записи сообщения, но так как она маловероятно при большом объеме буфера и малом числе клиентов, мы ее пропускаем

  // запись сообщения
  // if (verbose) printf("[write message %s...]\n", message->data.text);

  // производим копирование сообщение в буфер shared_memory
  message->type = type; // указываем тип сообщения
  message->data.kind = kind; // указываем вид сообщения
  memset(message->data.writer_name, '\0', sizeof(message->data.writer_name));
  strcpy(message->data.writer_name, writer_name); // указываем имя отправителя
  message->data.writer_id = writer_id; // указываем id отправителя
  memset(message->data.text, '\0', sizeof(message->data.text));
  strcpy(message->data.text, text); // указываем текст сообщения
  message->data.state = MESSAGE_SENT; // указываем состояние сообщения

  // if (verbose) printf("[write message %s ok!]\n", message->data.text);

  // if (verbose) printf("[write message unlock... mutex_sem]\n");
  // разблокируем mutex_sem для работы остальных клиентов
  asem[0].sem_op = 1;
  if (semop(shm_ctl.mutex_sem, asem, 1) == -1)
    error("[semop: mutex_sem]");

  // if (verbose) printf("[write message unlock... mutex_sem ok!]\n");
};

// получение сообщения из shared_memory
struct message_buffer read_message(struct shared_memory_control shm_ctl, int type) {

  // if (verbose) printf("[reader type -- %d]\n", type);

  // создаем инстанс структуры сообщения
  struct message_buffer message;
  message.data.state = MESSAGE_ERROR; // если сообщение не будет найдено в буфере, произойдет возврат ошибки

  // структура для управления состоянием семафора
  struct sembuf asem[1];
  asem[0].sem_num = 0;
  asem[0].sem_op = 0; // флаг операции над счетчиком семафора -1 вычитание из счетчика, 1 прибавление к счетчику
  asem[0].sem_flg = 0;

  // if (verbose) printf("[read message lock... mutex_sem]\n");
  // определяем есть ли еще сообщения для чтения по состоянию read_mutex
  // если значение семафора равно нулю, то либо сообщений не было, либо они все обработаны в данном цикле
  // соответственно ядро защититься, так как значение семафора не может быть меньше нуля
  // процесс будет заблокирован до тех пор, пока пишуший процесс не добавит новое сообщение в буфер и не разблокирует данный семафор
  // увеличив счетчик семафора на единицу (это нужно сделать после добавления сообщения в буфер)
  asem[0].sem_op = -1;
  if (semop(shm_ctl.mutex_sem, asem, 1) == -1)
    perror("[semop: mutex_sem]");
  // if (verbose) printf("[read message lock... mutex_sem ok!]\n");

  // пытаемся найти нужное нам сообщение в буфере разделяемой памяти
  for (int i = 0; i < MAX_MESSAGES; i++) {

    // получаем указатель на текущее сообщение в буффере
    struct message_buffer * message_ptr = & shm_ctl.shared_mem_ptr->messages[i];

    // проверяем является ли данное сообщение предназначенным для считывающего процесса
    // if (verbose) printf("[read message type %ld]\n", message_ptr->type);
    if (message_ptr->data.state == MESSAGE_SENT && message_ptr->type == type) {
      // if (verbose) printf("[read message type %ld ok!]\n", message_ptr->type);
      // if (verbose) printf("[read message %s]\n", message_ptr->data.text);

      // копируем сообщение в выходную структуру и переходим к следующему индексу для считывания
      message.type = message_ptr->type; // указываем тип сообщения
      message.data.kind = message_ptr->data.kind; // указываем вид сообщения
      memset(message.data.writer_name, '\0', sizeof(message.data.writer_name));
      strcpy(message.data.writer_name, message_ptr->data.writer_name); // указываем имя отправителя
      message.data.writer_id = message_ptr->data.writer_id; // указываем id отправителя
      memset(message.data.text, '\0', sizeof(message.data.text));
      strcpy(message.data.text, message_ptr->data.text); // указываем текст сообщения
      message.data.state = MESSAGE_RECEIVED; // указываем состояние сообщения

      message_ptr->data.state = MESSAGE_NONE; // обновляем состояние буфера сообщений
      // теперь возможна запись в данную ячейку буффера (только после разблокировки семафора mutex_sem)

      // if (verbose) printf("[read message %s ok!]\n", message.data.text);
      break; // считываем только одно сообщение за раз
    }
  }

  // if (verbose) printf("[read message unlock... mutex_sem]\n");
  // содержимое буффера было считано, теперь освобождаем буфер для дальнейшей записи сообщений
  // разблокируем семафор write_mutex для дальнейшего ввода сообщений клиентами
  asem[0].sem_op = 1;
  if (semop(shm_ctl.mutex_sem, asem, 1) == -1)
    perror("[semop: mutex_sem]");
  // if (verbose) printf("[read message unlock... mutex_sem ok!]\n");

  return message;
};

// функция установа типа сообщения с кодом получателя client_id
int set_addressee(int client_id) {
  int addressee = MESSAGE_MAGIC_NUMBER + client_id;
  return addressee;
};

struct message_data input(int time_limit) {

  if (verbose) printf("[wait input...]\n");

  struct message_data message;
  memset(message.text, '\0', sizeof(message.text));
  struct timeval tmo;
  fd_set readfds;

  fflush(stdout);

  FD_ZERO( & readfds);
  FD_SET(0, & readfds);
  tmo.tv_sec = time_limit;
  tmo.tv_usec = 0;

  switch (select(1, & readfds, NULL, NULL, & tmo)) {
  case -1:
    perror("[select error]");
    break;
  case 0:
    message.kind = -1;
    return message;
  }

  gets(message.text);
  if (strcmp(message.text, "") == 0) {
    message.kind = -1;
  } else {
    message.kind = 0;
  }

  if (verbose) printf("[wait input... ok!]\n");

  return message;
}

// прогамма запуска и работы сервера
static void run_server() {

  key_t s_key; // ключ доступа к структурам IPC
  union semun sem_attr; // структура для инициализации семафоров
  struct shared_memory_control shm_ctl; // структура для управления состоянием shared_memory
  int client_id_pool[MAX_CLIENTS]; // пул id клиентов

  //  инициализация мьютекса разделяемой памяти
  if ((s_key = ftok(SEM_MUTEX_KEY, PROJECT_ID)) == -1)
    error("[ftok]");
  if ((shm_ctl.mutex_sem = semget(s_key, 1, 0600 | IPC_CREAT | IPC_EXCL)) == -1)
    error("[semget]");
  // установ начального значения мьютекса
  sem_attr.val = 0; // заблокирован до завершения процесса инициализации
  if (semctl(shm_ctl.mutex_sem, 0, SETVAL, sem_attr) == -1)
    error("[semctl SETVAL]");

  // создание и получение доступа к сегменту разделяемой памяти
  if ((s_key = ftok(SHARED_MEMORY_KEY, PROJECT_ID)) == -1)
    error("[ftok]");
  if ((shm_ctl.shm_id = shmget(s_key, sizeof(struct shared_memory),
      0600 | IPC_CREAT | IPC_EXCL)) == -1)
    error("[shmget]");
  if ((shm_ctl.shared_mem_ptr = (struct shared_memory * ) shmat(shm_ctl.shm_id, NULL, 0)) ==
    (struct shared_memory * ) - 1)
    error("[shmat]");
  // инициализируем буфер сообщений
  for (int i = 0; i < MAX_MESSAGES; i++) {
    shm_ctl.shared_mem_ptr->messages[i].data.state = MESSAGE_NONE;
  }

  // инициализация массива идентификаторов
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_id_pool[i] = CLIENT_ID_FREE;
  }

  // инициализация завершена
  // разблокируем мьютекс доступа к разделяемой памяти для клиентов
  sem_attr.val = 1;
  if (semctl(shm_ctl.mutex_sem, 0, SETVAL, sem_attr) == -1)
    error("[semctl SETVAL]");

  // структура для управления состоянием семафора
  struct sembuf asem[1];
  asem[0].sem_num = 0;
  asem[0].sem_op = 0; // флаг операции над счетчиком семафора -1 вычитание из счетчика, 1 прибавление к счетчику
  asem[0].sem_flg = 0;

  printf("please type q or quit to exit the program... \n");

  while (!program_exit_flag) {

    /*
     * Обработка команды выхода из программы
     */
    if (verbose) printf("please type q or quit to exit the program... \n");
    struct message_data command_message = input(5);
    if (command_message.kind != -1){
      if(strcmp(command_message.text, "q") == 0 || strcmp(command_message.text, "quit") == 0) {
        break;
      }
    }

    /*
     * Регистрация клиента
     */
    if (verbose) printf("[register new clients...]\n");
    struct message_buffer message_register = read_message(shm_ctl, MESSAGE_CLIENT_ID_REQUEST);

    if (message_register.data.state == MESSAGE_RECEIVED) {

      if (verbose) printf("[register new clients... start...]\n");

      // производим регистрацию клиента
      for (int i = 0; i < MAX_CLIENTS; i++) {

        // берем первый свободный номер из пула идентификаторов клиентов
        if (client_id_pool[i] == CLIENT_ID_FREE) {

          // устанавливаем client_id равным не занятому индексу пула
          int client_id = i;

          // резервируем id для клиента в пуле
          client_id_pool[i] = CLIENT_ID_TAKED;

          // переводим client_id в char тип
          char client_id_text[MESSAGE_MAX_LENGTH] = "";
          snprintf(client_id_text, MESSAGE_MAX_LENGTH, "%d", client_id);

          if (verbose) printf("[send client_id to client to proof the registering...]\n");

          // отправляем подтверждение регистрации клиента с client_id = i
          write_message(shm_ctl, SERVER_NAME, SERVER_ID, MESSAGE_CLIENT_ID_SET, MESSAGE_KIND_MESSAGE, client_id_text);

          // выводим сообщение об успешном подтверждении регистрации
          if (verbose) printf("[client %s successfully registered with client_id = %d]\n", message_register.data.writer_name, client_id);

          // сообщение успешно отправлено, теперь нужно оповестить всех клиентов
          for (int j = 0; j < MAX_CLIENTS; j++) {

            if (j != client_id && client_id_pool[j] == CLIENT_ID_TAKED) {

              // отправляем сообщение подключения нового клиента к чату с его именем
              write_message(shm_ctl, SERVER_NAME, SERVER_ID, set_addressee(j), MESSAGE_KIND_IN, message_register.data.writer_name);

              // можно не делать проверку
            }
          }

          break; // выходим из цикла
          // если регистрация произошла с ошибкой, то клиент может заново потребовать регистрации
          // его сообщение будет обработано на следующей итерации цикла while
        }
      }

      if (verbose) printf("[register new clients... end.]\n");
      // назначаем уникальный идентификатор клиента client_id
    } else {

      if (verbose) printf("[register new clients... no clients to register!]\n");
    }

    /*
     * Обработка клиентских сообщений
     */
    if (verbose) printf("[process clients messages...]\n");
    struct message_buffer message = read_message(shm_ctl, MESSAGE_POST_REQUEST);

    if (message.data.state == MESSAGE_RECEIVED) {

      // отправка сообщений другим клиентам
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (message.data.writer_id != i && client_id_pool[i] == CLIENT_ID_TAKED) {

          if (verbose) printf("[process client message %d...]\n", i);

          // продублировать сообщение в очереди с указанием client_id (закодировано в типе сообщения)
          write_message(shm_ctl, message.data.writer_name, SERVER_ID, set_addressee(i), MESSAGE_KIND_MESSAGE, message.data.text);

          if (verbose) printf("[process client message %d... ok!]\n", i);

          // можно сделать проверку успешно ли доставлено сообщение
        }
      }
    }
    if (verbose) printf("[process clients messages... ok!]\n");

    /*
     * Обработка сообщения выхода из чата
     */
    if (verbose) printf("[process outchat messages...]\n");
    struct message_buffer message_out = read_message(shm_ctl, MESSAGE_OUT_CHAT);

    if (message_out.data.state == MESSAGE_RECEIVED) {

      // освобождение идентификатора клиента
      client_id_pool[message_out.data.writer_id] = CLIENT_ID_FREE;

      // оповещение всех клиентов о выходе клиента из чата
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_id_pool[i] == CLIENT_ID_TAKED) {

          // отправка сообщения выхода клиента из чата (text содержит имя клиента)
          write_message(shm_ctl, message.data.writer_name, SERVER_ID, set_addressee(i), MESSAGE_KIND_OUT, message_out.data.text);
        }
      }
    }

    if (verbose) printf("[process outchat messages... ok!]\n");

    // организуем небольшую задержку
    sleep(TIME_DELAY_CYCLE);
  }

  printf("Stop doing job...\n");

  if (verbose) printf("[messaging end job to all clients...]\n");

  // оповещение всех клиентов о завершении работы сервера
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (client_id_pool[i] == CLIENT_ID_TAKED) {

      // отправка сообщения выхода клиента из чата (text содержит имя клиента)
      write_message(shm_ctl, SERVER_NAME, SERVER_ID, set_addressee(i), MESSAGE_KIND_OUT, "end job");
    }
  }

  if (verbose) printf("[messaging end job to all clients... ok!]\n");

  sleep(TIME_WAIT_END_JOB); // ожидание заверешения работы клиентов

  if (verbose) printf("[deleting shm %d...]\n", shm_ctl.shm_id);

  // освобождение выделеной памяти shm_id
  shmctl(shm_ctl.shm_id, IPC_RMID, NULL);

  if (verbose) printf("[deleting shm %d... ok!]\n", shm_ctl.shm_id);

  if (verbose) printf("[deleting sem %d...]\n", shm_ctl.mutex_sem);

  // удаление семафора
  semctl(shm_ctl.mutex_sem, 0, IPC_RMID, 0);

  if (verbose) printf("[deleting sem %d... ok!]\n", shm_ctl.mutex_sem);

  printf("End program.\n");
};

// программа запуска и работы клиента
static void run_client() {

  key_t s_key; // ключ доступа к структурам IPC
  struct shared_memory_control shm_ctl; // структура для управления shared_memory
  union semun sem_attr; // структура для инициализации семафоров
  char client_name[MESSAGE_MAX_LENGTH]; // имя пользователя
  int client_id = -1; // id клиента
  int wait_client_id_flag = 0; // флаг ожидания id клиента

  // инициализация мьютекса управления потоками
  // производит взаимное исключение нахождения потоков в критической секции
  if ((s_key = ftok(SEM_MUTEX_KEY, PROJECT_ID)) == -1) // генерация уникального ключа из файлового дескриптора
    error("[ftok]");
  if ((shm_ctl.mutex_sem = semget(s_key, 1, 0)) == -1)
    error("[semget]");

  // получение доступа к разделяемой памяти
  if ((s_key = ftok(SHARED_MEMORY_KEY, PROJECT_ID)) == -1)
    error("[ftok]");
  if ((shm_ctl.shm_id = shmget(s_key, sizeof(struct shared_memory), 0)) == -1)
    error("[shmget]");
  if ((shm_ctl.shared_mem_ptr = (struct shared_memory * ) shmat(shm_ctl.shm_id, NULL, 0)) == (struct shared_memory * ) - 1)
    error("[shmat]");

  // ввод имени пользователя
  printf("please enter client name:\n");
  scanf("%s", (char * ) & client_name);
  printf("hello, %s!\n", client_name);

  while (!program_exit_flag) {

    // небольшая задержка
    sleep(TIME_DELAY_CYCLE);

    /*
     * Регистрация клиента
     */
    if (client_id == -1) {

      if (!wait_client_id_flag) {

        if (verbose) printf("[client request registering...]\n");

        // отправить запрос на получение идентификатора пользователя (client_id)
        write_message(shm_ctl, client_name, client_id, MESSAGE_CLIENT_ID_REQUEST, MESSAGE_KIND_IN, client_name);

        if (verbose) printf("[register request sent! waiting registering...]\n");

        wait_client_id_flag = 1;
      } else {

        if (verbose) printf("[wait registering...]\n");

        // получить сообщение с идентификатором пользователя
        struct message_buffer message_client_id = read_message(shm_ctl, MESSAGE_CLIENT_ID_SET);

        if (message_client_id.data.state == MESSAGE_RECEIVED) {

          client_id = atoi(message_client_id.data.text);

          if (verbose) printf("[register success! client_id = %d]\n", client_id);
        } else {
          if (verbose) printf("[client request registering... failed!]\n");
        }
      }
    }

    if (client_id == -1) continue;

    /*
     * Получение сообщений от клиентов
     */

    while (!program_exit_flag) {
      struct message_buffer message = read_message(shm_ctl, set_addressee(client_id));

      if (message.data.state == MESSAGE_RECEIVED) {

        // определение вида сообщения
        if (message.data.kind == MESSAGE_KIND_MESSAGE) {

          // простое сообщение для отображения
          printf("%s>> %s\n", message.data.writer_name, message.data.text);

        } else if (message.data.kind == MESSAGE_KIND_OUT) {

          // обработка сообщения выхода из чата
          if (strcmp(message.data.writer_name, SERVER_NAME) == 0) {

            // сообщение о завершении работы сервера
            printf("%s\n", SERVER_NAME);
            if (verbose) printf("[server %s]\n[stop client]\n", message.data.text);
            printf("Bye!");

            exit(EXIT_SUCCESS); // принудительное завершение работы клиента

          } else {

            // сообщение выхода клиента из чата
            printf("%s %s\n", message.data.text, "left the chat room");

          }
        } else if (message.data.kind == MESSAGE_KIND_IN) {

          // сообщение входа в чат
          printf("%s %s\n", message.data.text, "joined to the chat");
        }
      } else {

        break;
      }
    }

    /*
     * Отправка сообщений
     */
    char text[MESSAGE_MAX_LENGTH];

    // простой пользовательский ввод (без ограничения на время)
    // printf(">> ");
    // gets(message_data.text);

    // пользовательский ввод с ограничением по времени (5 секунд)
    if (verbose) printf("please type your message...\n");
    struct message_data message_data = input(5);

    if (message_data.kind != -1) {

      memset(text, '\0', sizeof(text));
      strcpy(text, message_data.text);

      if (strcmp(text, "q") == 0 || strcmp(text, "quit") == 0) {

        break;

      } else {

        // запрос на отправку сообщения
        write_message(shm_ctl, client_name, client_id, MESSAGE_POST_REQUEST, MESSAGE_KIND_MESSAGE, text);

        // здесь можно проверить была ли отправка успешной
      }
    }
  }

  // выход из чата
  write_message(shm_ctl, client_name, client_id, MESSAGE_OUT_CHAT, MESSAGE_KIND_OUT, client_name);
};

// отображение справки
static void print_help(char * program_name, char * message) {
  if (message != NULL) fputs(message, stderr);
  fprintf(stderr, "Usage: %s [options]\n", program_name);
  fprintf(stderr, "Options are:\n");
  fprintf(stderr, "-c client mode read/send messages (by default)\n");
  fprintf(stderr, "-s server mode control messages/clients\n");
  fprintf(stderr, "-v verbosity level: 0 just chat, 1 explain job client-server\n");
  exit(EXIT_FAILURE);
};

// обработка выхода из программы
void exit_program() {
  program_exit_flag = 1;
};

// точка входа в программу
int main(int argc, char * argv[]) {

  // установ сигнала выхода из программы по команде ctrl-c из терминала
  signal(SIGINT, exit_program);

  int mode = DEFAULT_MODE; // режим работы программы: 0 - клиент, 1 - сервер

  // читаем параметры аргументов командной строки
  int opt;
  while ((opt = getopt(argc, argv, "csv")) != -1) {
    switch (opt) {
      // опция клиента
    case 'c':
      mode = 0;
      break;
      // опция сервера
    case 's':
      mode = 1;
      break;
      // опция болтать
    case 'v':
      verbose = 1;
      break;
    default:
      print_help(argv[0], "Unrecognized option\n");
    }
  }

  // запускаем сервер или клиент
  if (mode == 1) {
    run_server();
  } else {
    run_client();
  }

  // выход из программы с флагом успешного завершения
  exit(EXIT_SUCCESS);
}

// TODO служебное сообщение по текущим клиентам в чате
// TODO многопоточность
