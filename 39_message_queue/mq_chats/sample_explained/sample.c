/*
 * Вариант System V
 *
 * Программа для тестирования создания очереди, записи и чтения сообщений
 *
 * Дополнительная информация: http://asu.cs.nstu.ru/~evgen/man/
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

// определение параметров по умолчанию
#define DEFAULT_MESSAGE_TEXT "Hello, World!" // стандартный текст сообщения
#define MESSAGE_MAX_LENGTH 128 // длина сообщения в байтах по умолчанию
#define DEFAULT_KEY 1234 // ключ для создания очереди сообщений по умолчанию
#define DEFAULT_MESSAGE_TYPE 1 // тип сообщения по умолчанию
#define DEFAULT_MODE 0 // режим неопределен

// структура сообщения
struct message_buffer {
    long message_type; // тип сообщения - любое положительное число
    char message_text[MESSAGE_MAX_LENGTH]; // текст сообщения
};

// отправка сообщения в очередь
static void send_message(int queue_id, int message_type, char *message_text) {

    // создаем инстанс сообщения
    struct message_buffer message;

    // заполняем данные сообщения
    message.message_type = message_type; // указываем тип сообщения
    for(int i = 0; i < strlen(message_text); i++) {
      message.message_text[i] = message_text[i]; // указываем текст сообщения
    }

    // пытаемся отправить сообщение в очередь, проверяем ошибку
    if (msgsnd(queue_id, (void *) &message, sizeof(message.message_text), IPC_NOWAIT) == -1) {

      // показываем ошибку, выходим из программы с кодом завершения с ошибкой
      perror("msgsnd error");
      exit(EXIT_FAILURE);
    }

    // сообщение успешно отправлено в очередь
    printf("message sent: %s\n", message.message_text);
}

// получение сообщения из очереди
static void receive_message(int queue_id, int message_type) {

    // создаем инстанс структуры сообщения
    struct message_buffer message;

    // пытаемся получить сообщение из очереди в наш инстанс, проверяем ошибки
    if (msgrcv(queue_id, (void *) &message, sizeof(message.message_text), message_type, MSG_NOERROR | IPC_NOWAIT) == -1) {

      // это не ошибка отсуствия сообщения?
      if (errno != ENOMSG) {
        // нет это другая ошибка
        perror("msgrcv error");
        exit(EXIT_FAILURE);
      }

      // ошибка отсутствия сообщения в очереди
      printf("No message available for msgrcv()\n");
    }

    // сообщение успешно получено из очереди
    printf("message received: %s\n", message.message_text);
}

// отображение справки
static void print_help(char *program_name, char *message) {
    if (message != NULL) fputs(message, stderr);
    fprintf(stderr, "Usage: %s [options]\n", program_name);
    fprintf(stderr, "Options are:\n");
    fprintf(stderr, "-s send message using msgsnd()\n");
    fprintf(stderr, "-r read message using msgrcv()\n");
    fprintf(stderr, "-t message type (default is 1)\n");
    fprintf(stderr, "-m message text (default <<Hello, World!>>)\n");
    fprintf(stderr, "-k message queue key (default is 1234)\n");
    exit(EXIT_FAILURE);
}

// точка входа в программу
int main(int argc, char *argv[]) {

    int queue_id; // идентификатор очереди сообщений
    int key = DEFAULT_KEY; // ключ для создания очереди
    int mode = DEFAULT_MODE; // режим работы программы: 1 - отправка сообщения в очередь, 2 - получение сообщения из очереди
    int message_type = DEFAULT_MESSAGE_TYPE; // тип сообщения
    char message_text[MESSAGE_MAX_LENGTH] = DEFAULT_MESSAGE_TEXT; // текст сообщения

    // читаем параметры аргументов командной строки
    int opt;
    while ((opt = getopt(argc, argv, "srt:m:k:")) != -1) {
      switch (opt) {
        // опция отправки
        case 's':
          mode = 1;
          break;
        // опция чтения
        case 'r':
          mode = 2;
          break;
        // опция тип сообщения
        case 't':
          message_type = atoi(optarg);
          if (message_type <= 0)
            print_help(argv[0], "-t option must be greater than 0\n");
          break;
        // опция текст сообщения
        case 'm':
          if (strlen(optarg) > MESSAGE_MAX_LENGTH) {
            // обработка ошибки превышения максимальной длины сообщения
            char error_message[MESSAGE_MAX_LENGTH];
            sprintf(error_message, "length of message should be lesser or equal %d\n", MESSAGE_MAX_LENGTH);
            print_help(argv[0], error_message);
          }
          // запись сообщения в массив
          snprintf(message_text, MESSAGE_MAX_LENGTH, "%s", optarg);
          break;
        // опция ключ (для создания очереди)
        case 'k':
          key = atoi(optarg);
          break;
        default:
          print_help(argv[0], "Unrecognized option\n");
      }
    }

    // проверка установа правильного значения режима работы программы (должен быть равен 1 или 2)
    if (mode == 0) print_help(argv[0], "must use either -s or -r option\n");

    // получение идентификатора очереди сообщений (очередь создается с правами доступа чтения-записи)
    // IPC_CREAT указывает системе создать новую очередь сообщений
    // 0666 устанавливает права доступа к очереди сообщений
    // -rw-rw-rw- (разрешение на чтение-запись)
    queue_id = msgget(key, IPC_CREAT | 0666);
    if (queue_id == -1) {
      perror("msgget error");
      exit(EXIT_FAILURE);
    }

    // получен queue_id идентификатор очереди
    printf("msgget() call success! queue_id received %d\n", queue_id);

    // выполнение операций отправки/получения сообщения
    if(mode == 1) send_message(queue_id, message_type, message_text);
    else receive_message(queue_id, message_type);

    // выход из программы с флагом успешного завершения
    exit(EXIT_SUCCESS);
}
