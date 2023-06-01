/* Вариант System V */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// определение параметров по умолчанию
#define MESSAGE_MAX_LENGTH 128 // длина сообщения в байтах по умолчанию
#define DEFAULT_KEY 1234 // ключ для создания очереди сообщений по умолчанию
#define DEFAULT_MODE 0 // 0 клиент, 1 сервер
#define MAX_CLIENTS 10 // максимальное количество клиентов для обслуживания сервером

// возможные типы сообщений
#define MESSAGE_CLIENT_ID_REQUEST 1
#define MESSAGE_CLIENT_ID_SET 2
#define MESSAGE_POST_REQUEST 3
#define MESSAGE_OUT_CHAT 4
#define MESSAGE_MAGIC_NUMBER 5

// идентификатор и имя сервера
#define SERVER_ID MAX_CLIENTS + 41
#define SERVER_NAME "MAIN QUESTION ANSWER IS 42"

// временные константы (с)
#define TIME_DELAY_CYCLE 1 // время задерки между циклами обработки сообщений
#define TIME_WAIT_END_JOB 15 // время ожидания завершения программы

// возможные виды сообщений
#define MESSAGE_KIND_MESSAGE 0
#define MESSAGE_KIND_OUT 1
#define MESSAGE_KIND_IN 2

// возможные состояния сообщений
#define MESSAGE_PREPARED 0
#define MESSAGE_RECEIVED 1
#define MESSAGE_SENT 2
#define MESSAGE_ERROR - 1

// возможные состояния client_id
#define CLIENT_ID_FREE 0
#define CLIENT_ID_TAKED 1

// флаг обработки выхода из программы
static volatile int program_exit_flag = 0;
static volatile int verbose = 0;

// структура сообщения
struct message_data {
  long kind; // вид сообщения
  long state; // состояние сообщения
  char sender_name[MESSAGE_MAX_LENGTH]; // имя отправителя
  long sender_id; // id отправителя
  char text[MESSAGE_MAX_LENGTH]; // текст сообщения
};

struct message_buffer {
  long type; // тип сообщения (обязательный параметр типа long, все остальные параметры произвольные)
  struct message_data data;
};

// отправка сообщения в очередь
struct message_buffer send_message(int queue_id, char * sender_name, int sender_id, int type, int kind, char * text) {

  // создаем инстанс сообщения
  struct message_buffer message;

  // заполняем данные сообщения
  message.type = type; // указываем тип сообщения
  message.data.kind = kind; // указываем вид сообщения
  message.data.state = MESSAGE_PREPARED; // указываем состояние сообщения
  memset(message.data.sender_name, '\0', sizeof(message.data.sender_name));
  strcpy(message.data.sender_name, sender_name); // указываем имя отправителя
  message.data.sender_id = sender_id; // указываем id отправителя
  memset(message.data.text, '\0', sizeof(message.data.text));
  strcpy(message.data.text, text); // указываем текст сообщения

  // printf("message_type = %d\n", message.type);
  // printf("message_kind = %d\n", message.data.kind);
  // printf("message_state = %d\n", message.data.state);
  // printf("message_sender_name = %s\n", message.data.sender_name);
  // printf("message_sender_id = %d\n", message.data.sender_id);
  // printf("message_text = %s\n", message.data.text);

  // пытаемся отправить сообщение в очередь, проверяем ошибку
  if (msgsnd(queue_id, & message, sizeof(message.data), IPC_NOWAIT) == -1) {

    // показываем ошибку, выходим из программы с кодом завершения с ошибкой
    perror("msgsnd error");

    // устанавливаем флаг ошибки на состояние
    message.data.state = MESSAGE_ERROR;

  } else {

    // сообщение успешно отправлено в очередь
    if (verbose) printf("[message sent: %s]\n", message.data.text);

    // устанавливаем флаг успешной отправки сообщения
    message.data.state = MESSAGE_SENT;
  }

  return message;
};

// получение сообщения из очереди
struct message_buffer receive_message(int queue_id, int type) {

  // создаем инстанс структуры сообщения
  struct message_buffer message;

  // пытаемся получить сообщение из очереди в наш инстанс, проверяем ошибки
  if (msgrcv(queue_id, (void * ) & message, sizeof(message.data), type, MSG_NOERROR | IPC_NOWAIT) == -1) {

    // это не ошибка отсуствия сообщения?
    if (errno != ENOMSG) {
      // нет это другая ошибка
      perror("msgrcv error");
    } else {
      // ошибка отсутствия сообщения в очереди
      //printf("No message available for msgrcv()\n");
    }

    // устанавливаем флаг ошибки на состояние
    message.data.state = MESSAGE_ERROR;
  } else {

    // сообщение успешно получено из очереди
    if (verbose) printf("[message received: %s]\n", message.data.text);

    // устанавливаем флаг успешного получения сообщения
    message.data.state = MESSAGE_RECEIVED;
  }

  return message;
};

// отображение справки
static void print_help(char * program_name, char * message) {
  if (message != NULL) fputs(message, stderr);
  fprintf(stderr, "Usage: %s [options]\n", program_name);
  fprintf(stderr, "Options are:\n");
  fprintf(stderr, "-c client mode read/send messages (by default)\n");
  fprintf(stderr, "-s server mode control messages/clients\n");
  fprintf(stderr, "-k message queue key (default is 1234)\n");
  exit(EXIT_FAILURE);
};

// функция установа типа сообщения с кодом получателя client_id
int set_addressee(int client_id) {
  int addressee = MESSAGE_MAGIC_NUMBER + client_id;
  return addressee;
};

struct message_data input(int time_limit) {

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
    perror("select error");
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

  return message;
}

// прогамма запуска и работы сервера
static void run_server(int key) {

  int queue_id;
  int client_id_pool[MAX_CLIENTS];

  // инициализация массива идентификаторов
  for (int i = 0; i < MAX_CLIENTS; i++) {
    client_id_pool[i] = CLIENT_ID_FREE;
  }

  // СОЗДАНИЕ ОЧЕРЕДИ СООБЩЕНИЙ
  // получение идентификатора очереди сообщений (очередь создается с правами доступа чтения-записи)
  // IPC_EXCL указывает не создавать очередь, если она существует и сообщить ошибку
  // IPC_CREAT указывает системе создать новую очередь сообщений
  // 0600 устанавливает права доступа к очереди сообщений
  // -rw------- (разрешение на чтение-запись только для пользователя)
  queue_id = msgget(key, IPC_EXCL | IPC_CREAT | 0600);
  if (queue_id == -1) {
    perror("msgget error");
    exit(EXIT_FAILURE);
  }

  // получен queue_id идентификатор очереди
  if (verbose) printf("[msgget() call success! queue_id received %d]\n", queue_id);

  printf("type q or quit to exit the program... \n");

  while (!program_exit_flag) {

    /*
     * Обработка команды выхода из программы
     */
    if (verbose) printf("type q or quit to exit the program... \n");
    struct message_data command_message = input(5);
    if (command_message.kind != -1) {
      if (strcmp(command_message.text, "q") == 0 || strcmp(command_message.text, "quit") == 0) {
        break;
      }
    }

    /*
     * Регистрация клиента
     */
    struct message_buffer message_register = receive_message(queue_id, MESSAGE_CLIENT_ID_REQUEST);

    if (message_register.data.state == MESSAGE_RECEIVED) {

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

          // отправляем подтверждение регистрации клиента с client_id = i
          struct message_buffer message_approve = send_message(queue_id, SERVER_NAME, SERVER_ID, MESSAGE_CLIENT_ID_SET, MESSAGE_KIND_MESSAGE, client_id_text);

          if (message_approve.data.state == MESSAGE_SENT) {

            // выводим сообщение об успешном подтверждении регистрации
            if (verbose) printf("[client %s successfully registered with client_id = %d]\n", message_register.data.sender_name, client_id);

            // сообщение успешно отправлено, теперь нужно оповестить всех клиентов
            for (int j = 0; j < MAX_CLIENTS; j++) {

              if (j != client_id && client_id_pool[j] == CLIENT_ID_TAKED) {

                // отправляем сообщение подключения нового клиента к чату с его именем
                send_message(queue_id, SERVER_NAME, SERVER_ID, set_addressee(j), MESSAGE_KIND_IN, message_register.data.sender_name);

                // можно не делать проверку
              }
            }
          }

          break; // выходим из цикла
          // если регистрация произошла с ошибкой, то клиент может заново потребовать регистрации
          // его сообщение будет обработано на следующей итерации цикла while
        }
      }
      // назначаем уникальный идентификатор клиента client_id
    }

    /*
     * Обработка клиентских сообщений
     */
    struct message_buffer message = receive_message(queue_id, MESSAGE_POST_REQUEST);

    if (message.data.state == MESSAGE_RECEIVED) {

      // отправка сообщений другим клиентам
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (message.data.sender_id != i && client_id_pool[i] == CLIENT_ID_TAKED) {

          // продублировать сообщение в очереди с указанием client_id (закодировано в типе сообщения)
          send_message(queue_id, message.data.sender_name, SERVER_ID, set_addressee(i), MESSAGE_KIND_MESSAGE, message.data.text);

          // можно сделать проверку успешно ли доставлено сообщение
        }
      }
    }

    /*
     * Обработка сообщения выхода из чата
     */
    struct message_buffer message_out = receive_message(queue_id, MESSAGE_OUT_CHAT);

    if (message_out.data.state == MESSAGE_RECEIVED) {

      // освобождение идентификатора клиента
      client_id_pool[message_out.data.sender_id] = CLIENT_ID_FREE;

      // оповещение всех клиентов о выходе клиента из чата
      for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_id_pool[i] == CLIENT_ID_TAKED) {

          // отправка сообщения выхода клиента из чата (text содержит имя клиента)
          send_message(queue_id, message.data.sender_name, SERVER_ID, set_addressee(i), MESSAGE_KIND_OUT, message_out.data.text);
        }
      }
    }

    // организуем небольшую задержку
    sleep(TIME_DELAY_CYCLE);
  }

  printf("Stop doing job...\n");

  if (verbose) printf("[messaging end job to all clients...]\n");

  // оповещение всех клиентов о завершении работы сервера
  for (int i = 0; i < MAX_CLIENTS; i++) {
    if (client_id_pool[i] == CLIENT_ID_TAKED) {

      // отправка сообщения завершения работы сервера
      send_message(queue_id, SERVER_NAME, SERVER_ID, set_addressee(i), MESSAGE_KIND_OUT, "end job");
    }
  }

  if (verbose) printf("[messaging end job to all clients... Ok!]\n");


  sleep(TIME_WAIT_END_JOB); // ожидаем 15 секунд для нормального заверешения работы клиентов

  if (verbose) printf("[deleting queue %d...]\n", queue_id);

  // удаляем очередь по выходу из цикла, освобождаем ресурс ядра
  msgctl(queue_id, IPC_RMID, NULL); // флаг IPC_RMID определяет операцию удаления очереди

  if (verbose) printf("[deleting queue %d... Ok!]\n", queue_id);

  printf("End program.\n");
};

// программа запуска и работы клиента
static void run_client(int key) {

  int queue_id;
  char client_name[MESSAGE_MAX_LENGTH];
  int client_id = -1;
  int wait_client_id_flag = 0;

  // получение идентификатора очереди сообщений (очередь создается с правами доступа чтения-записи)
  queue_id = msgget(key, 0); // получаем доступ к уже существующей очереди
  if (queue_id == -1) {
    perror("msgget error");
    exit(EXIT_FAILURE);
  }

  // получен queue_id идентификатор очереди
  if (verbose) printf("[msgget() call success! queue_id received %d]\n", queue_id);

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

        // отправить запрос на получение идентификатора пользователя (client_id)
        struct message_buffer message_register = send_message(queue_id, client_name, client_id, MESSAGE_CLIENT_ID_REQUEST, MESSAGE_KIND_IN, client_name);

        if (message_register.data.state == MESSAGE_SENT) {

          if (verbose) printf("[register request sent! waiting registering...]\n");

          wait_client_id_flag = 1;
        }
      }

      if (wait_client_id_flag) {

        // получить сообщение с идентификатором пользователя
        struct message_buffer message_client_id = receive_message(queue_id, MESSAGE_CLIENT_ID_SET);

        if (message_client_id.data.state == MESSAGE_RECEIVED) {

          client_id = atoi(message_client_id.data.text);

          if (verbose) printf("[register success! client_id = %d]\n", client_id);
        }
      }
    }

    if (client_id == -1) continue;

    /*
     * Получение сообщений от клиентов
     */
    while (!program_exit_flag) {

      struct message_buffer message = receive_message(queue_id, set_addressee(client_id));

      if (message.data.state == MESSAGE_RECEIVED) {

        // определение вида сообщения
        if (message.data.kind == MESSAGE_KIND_MESSAGE) {

          // простое сообщение для отображения
          printf("%s says %s\n", message.data.sender_name, message.data.text);

        } else if (message.data.kind == MESSAGE_KIND_OUT) {

          // обработка сообщения выхода из чата
          if (strcmp(message.data.sender_name, SERVER_NAME) == 0) {

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
          printf("%s %s\n", message.data.text, "joined to chat");
        }

      } else {

        break;
      }
    }

    /*
     * Отправка сообщений
     */
    char text[MESSAGE_MAX_LENGTH];

    // печатем инвайт
    if (verbose) printf("type your message...\n");

    // простой пользовательский ввод (без ограничения на время)
    // gets(text);

    // пользовательский ввод с ограничением по времени (5 секунд)
    struct message_data message_data = input(5);

    if (message_data.kind != -1) {

      memset(text, '\0', sizeof(text));
      strcpy(text, message_data.text);

      if (strcmp(text, "q") == 0 || strcmp(text, "quit") == 0) {

        break; // выходим из цикла

      } else if (strcmp(text, "r") == 0 || strcmp(text, "recive") == 0) {

        continue;

      } else {

        // запрос на отправку сообщения
        send_message(queue_id, client_name, client_id, MESSAGE_POST_REQUEST, MESSAGE_KIND_MESSAGE, text);

        // здесь можно проверить была ли отправка успешной
      }
    }
  }

  // выход из чата
  send_message(queue_id, client_name, client_id, MESSAGE_OUT_CHAT, MESSAGE_KIND_OUT, client_name);
};

void exit_program() {
  program_exit_flag = 1;
};

// точка входа в программу
int main(int argc, char * argv[]) {

  // установ сигнала выхода из программы по команде ctrl-c из терминала
  signal(SIGINT, exit_program);

  int queue_id; // идентификатор очереди сообщений
  int key = DEFAULT_KEY; // ключ для создания очереди
  int mode = DEFAULT_MODE; // режим работы программы: 0 - клиент, 1 - сервер

  // читаем параметры аргументов командной строки
  int opt;
  while ((opt = getopt(argc, argv, "csk:v")) != -1) {
    switch (opt) {
    // опция клиента
    case 'c':
      mode = 0;
      break;
    // опция сервера
    case 's':
      mode = 1;
      break;
    // опция ключ (для создания очереди)
    case 'k':
      key = atoi(optarg); // по идее тут должен быть путь к временному файлу-дескриптору и генерация ключа функцией ftok, но мы просто используем int значение ключа, так как это быстрее и удобнее для теста
      break;
    // опция болтать (выводить служебные сообщения о состояниях клиента и сервера)
    case 'v':
      verbose = 1;
      break;
    default:
      print_help(argv[0], "Unrecognized option\n");
    }
  }

  if (mode == 1) {
    run_server(key);
  } else {
    run_client(key);
  }

  // выход из программы с флагом успешного завершения
  exit(EXIT_SUCCESS);
}

// TODO служебное сообщение по текущим клиентам в чате
// TODO многопоточность
