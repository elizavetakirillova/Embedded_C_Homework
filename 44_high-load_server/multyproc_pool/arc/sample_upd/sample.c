#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <signal.h>
#include <stdlib.h>
#include <limits.h>

#define DEFAULT_MODE 0 // 0 - клиент, 1 - сервер
#define DEFAULT_IP_ADDRESS INADDR_LOOPBACK // 127.0.0.1 по умолчанию
#define DEFAULT_PORT_NUMBER 8888 // для использования порта нужно ввести команду ufw allow 8888
#define UDP_HEADER_SIZE 8 // стандартный размер заголовка UPD-пакета
#define MESSAGE_MAX_LENGTH 32 // длина полезного сообщения (датаграммы)
#define UDP_MESSAGE_LENGHT UDP_HEADER_SIZE +  sizeof(char) * MESSAGE_MAX_LENGTH
#define SAMPLE_MESSAGE "Hello!\n" // сообщение для теста

// структура UDP-заголовка
struct udp_header{
		unsigned short source_port;  /* порт источника */
		unsigned short dest_port;    /* порт назначения */
		unsigned short packet_len;   /* длина UDP-пакета = 8 байт (заголовок) + MESSAGE_MAX_LENGTH (сообщение) */
		unsigned short check_sum;    /* чек-сумма для проверки целостности пакета = 0 (мы не проверяем)*/
};

// структура UDP-сообщения
struct upd_message {
	struct udp_header udph;
	char data[MESSAGE_MAX_LENGTH];
};

// флаги управления работой программы
static volatile int program_exit_flag = 0; // флаг обработки выхода из программы
static volatile int verbose = 0; // флаг управления выводом служебных сообщений

// функция запуска клиента
void run_client(unsigned short port_number){

	printf("I'm a client!\n");

	char buf_rec[MESSAGE_MAX_LENGTH];
	struct upd_message udp_msg;
	struct sockaddr_in svaddr;
	int sfd;

	/* Создаем клиентский сокет; привязываем его к уникальному пути
	(основанному на PID) */
	sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (sfd == -1)
		perror("perror");


	/* Формируем адрес сервера */
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(port_number);
	svaddr.sin_addr.s_addr = htonl(DEFAULT_IP_ADDRESS); // inet_addr("127.0.0.1");

	/* UDP-заголовок */
	udp_msg.udph.source_port=htons(port_number); // Порт отправителя
	udp_msg.udph.dest_port=htons(port_number); // Порт получателя
	udp_msg.udph.packet_len=htons(UDP_MESSAGE_LENGHT); // Длина заголовка
	udp_msg.udph.check_sum=0; // Контрольная сумма
	strcpy(&udp_msg.data, SAMPLE_MESSAGE);

	/* Копируем в сокет содержимое буфера запроса */
	if (sendto(sfd, &udp_msg, UDP_MESSAGE_LENGHT, 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("sendto");
	else
		printf("OK!\n");

	// здесь выключен режим приема сообщений, так как нам достаточно проверить прием на стороне сервера
	// // while(!program_exit_flag) {
	// 	if (recvfrom(sfd, buf_rec, MESSAGE_MAX_LENGTH, 0, NULL, NULL) == -1)
	// 		perror("recvfrom");
	// }

	// printf("%s", buf_rec);

	// обязательно закрываем сокет
	close(&svaddr);
};

// функция запуска сервера
void run_server(unsigned short port_number){

	printf("I'm a server!\n");

	struct sockaddr_in svaddr, claddr;
	int sfd, cfd;
	ssize_t numBytes;
	socklen_t len;
	char buf[MESSAGE_MAX_LENGTH];

	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd == -1)
		perror("socket");

	/* Формируем общеизвестный адрес сервера и привязываем к нему серверный сокет */
	/*if (remove(SV_SOCK_PATH) == -1)
		perror("remove");*/

	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(port_number);
	svaddr.sin_addr.s_addr = htonl(DEFAULT_IP_ADDRESS);

	if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("bind");

	/* Принимаем сообщения */
	while(!program_exit_flag) {

		len = sizeof(struct sockaddr_in);
		numBytes = recvfrom(sfd, buf, MESSAGE_MAX_LENGTH, 0, (struct sockaddr*) &claddr, &len);
		if (numBytes == -1)
			perror("recvfrom");

		printf("data: %s", buf);
		fflush(stdout);

  	// здесь выключена отправка сообщения, так как нам достаточно проверить отправку на стороне клиента
		// 	buf[0] = 'W';
		// 	if (sendto(sfd, buf, numBytes, 0, (struct sockaddr*) &claddr, len) == -1)
		// 			perror("sendto");
	}

	// обязательно закрываем сокет, чтобы сокет снова стал доступен для использования
	close(&svaddr);
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
	unsigned int port_number = DEFAULT_PORT_NUMBER;

  // читаем параметры аргументов командной строки
  int opt;
  while ((opt = getopt(argc, argv, "csp:v")) != -1) {
    switch (opt) {
      // опция клиента
    case 'c':
      mode = 0;
      break;
      // опция сервера
    case 's':
      mode = 1;
      break;
			// выбор порта
		case 'p':
	     if (atoi(optarg) >= 0 && atoi(optarg) <= USHRT_MAX)
				 port_number = atoi(optarg);
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
		printf("Run server on port %hu\n", port_number);
    run_server(port_number);
  } else {
		printf("Run client on port %hu\n", port_number);
    run_client(port_number);
  }

  // выход из программы с флагом успешного завершения
  exit(EXIT_SUCCESS);
}
