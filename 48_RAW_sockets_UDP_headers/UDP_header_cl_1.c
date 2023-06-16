/* Демонстрация UDP-заголовка для обмена пакетами по сети
клиента с сервером */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <pthread.h>
#include <limits.h>
#include <errno.h>
#include <err.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <netinet/in.h>
#include <netinet/udp.h>	//Provides declarations for udp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>
#include <arpa/inet.h>

#define DEFAULT_SERVER_IP INADDR_LOOPBACK // IP адрес сервера по умолчанию (127.0.0.1)
#define DEFAULT_PORT_SERVER 8000 // номер порта сервера по умолчанию (8000)
#define DEFAULT_PORT_CLIENT 8001 // номер порта клиента по умолчанию (8001)

// параметры сервера
#define SERVICE_ID_NOT_DEFINED USHRT_MAX

// параметры клиента
#define RECEIVE_MESSAGE_MAX_ATTEMPTS 5

// параметры UDP сообщения
#define UDP_HEADER_SIZE 8 // стандартный размер заголовка UPD-пакета в байтах
#define MESSAGE_DATA_LENGTH 20 // размер данных сообщения в байтах (sizeof char = 1 байт)
#define MESSAGE_FULL_LENGTH MESSAGE_DATA_LENGTH + 8 // полная длина сообщения в байтах (sizeof unsigned short = 2 байта)
#define UDP_MESSAGE_LENGTH UDP_HEADER_SIZE + MESSAGE_FULL_LENGTH // полная длина UDP-сообщения пересылаемого по сети
#define IP_PLUS_UDP_SIZE 20 + 8
#define RAW_MESSAGE_LEN IP_PLUS_UDP_SIZE + MESSAGE_FULL_LENGTH

// состояния сообщения
#define MESSAGE_PREPARED 0
#define MESSAGE_RECEIVED 1
#define MESSAGE_SENT 2
#define MESSAGE_ERROR 3

// типы сообщения
#define NO_MESSAGE 0
#define MESSAGE_CONNECT 1
#define MESSAGE_CLOSE 2
#define MESSAGE_DATA 3

// структура UDP-заголовка
struct udp_header {
	unsigned short source_port;  /* порт источника */
	unsigned short dest_port;    /* порт назначения */
	unsigned short packet_len;   /* длина UDP-пакета = 8 байт (заголовок) + MESSAGE_FULL_LENGTH (сообщение) */
	unsigned short check_sum;    /* чек-сумма для проверки целостности пакета = 0 (мы не проверяем)*/
};

// структура сообщения
struct message {
	unsigned short sid; // id привязанного сервиса
	unsigned short mid; // id сообщения
	unsigned short type; // тип сообщения (0 запрос подключения, 1 запрос отключения, 2 передача данных)
	unsigned short state; // состояние сообщения (0 подготовлено, 1 получено, 2 отправлено, 3 ошибка)
	char data[MESSAGE_DATA_LENGTH]; // данные сообщения
};

// структура UDP-сообщения
struct udp_message {
	struct udp_header udph;
	struct message msg;
};

// структура сообщения с адресом отправителя
struct message_ext {
	struct sockaddr_in addr;
	struct message msg;
};

struct message_raw {
	char buf[IP_PLUS_UDP_SIZE];
	struct message msg;
};

// получение raw UDP сообщения из сокета
struct message receive_message_reply(int socket_fd, struct sockaddr_in address) {
	struct message_raw message_raw;
	memset(&message_raw.msg.data, '\0', MESSAGE_DATA_LENGTH);
	socklen_t addr_len = sizeof(struct sockaddr_in);
	
	ssize_t number_of_bytes = recvfrom(socket_fd, &message_raw, RAW_MESSAGE_LEN , 0, (struct sockaddr*) &address, &addr_len);
	
	if (number_of_bytes == -1) {
		message_raw.msg.state = MESSAGE_ERROR;
		perror("recvfrom");
	} else {
		message_raw.msg.state = MESSAGE_RECEIVED;
	}
	return message_raw.msg;
}

int main(int argc, char* argv[])
{
	struct sockaddr_in svaddr; // claddr;
	int sfd;
	int sid = SERVICE_ID_NOT_DEFINED;
	int mid = 0;
	int type = MESSAGE_CONNECT;
	
	/* Создаем клиентский сокет; привязываем его к уникальному пути
	(основанному на PID) */
	sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (sfd == -1)
		perror("perror");

	/* UDP-заголовок */
	struct udp_message udp_message; 
	//struct udphdr* udph = (struct udphdr*) udp_message.msg.data;

	/* Формируем адрес сервера */
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(DEFAULT_PORT_SERVER);
	svaddr.sin_addr.s_addr = htonl(DEFAULT_SERVER_IP); // inet_addr("127.0.0.1"); // адрес сервера
	
	// формируем UPD заголовок сообщения
	udp_message.udph.source_port=htons(DEFAULT_PORT_CLIENT); // Порт отправителя
	udp_message.udph.dest_port=htons(DEFAULT_PORT_SERVER); // Порт получателя
	udp_message.udph.packet_len=htons(UDP_MESSAGE_LENGTH); // Длина пакета
	udp_message.udph.check_sum=0; // Контрольная сумма (не проверяем)

	/* UDP-заголовок
	udph->source = htons(DEFAULT_PORT_CLIENT); // вымышленный порт
	udph->dest = htons(DEFAULT_PORT_SERVER); // порт сервера
	udph->len = htons(UDP_MESSAGE_LENGTH);	// размер UDP заголовка
	udph->check = 0; // ставим 0 */

	/* Подготавливаем данные к отправке */
	memset(&udp_message.msg.data, '\0', MESSAGE_DATA_LENGTH);
	strcpy(udp_message.msg.data, "Hello!");
	udp_message.msg.type = type;
	udp_message.msg.sid = sid;
	udp_message.msg.mid = mid;
	udp_message.msg.state = MESSAGE_PREPARED;
	
	if (sendto(sfd, &udp_message, UDP_MESSAGE_LENGTH, 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("sendto");
	else
		printf("Packet Send. Length: %d \n", udp_message.udph.packet_len);

	/* Получение RAW UDP сообщения из сокета */
	int attempts = 0;
	struct message message;
	
	while(attempts < RECEIVE_MESSAGE_MAX_ATTEMPTS) {
		message = receive_message_reply(sfd, svaddr);
		if ((sid == SERVICE_ID_NOT_DEFINED || message.sid == sid) && message.mid == mid) {
			printf("%s\n", message.data);
    	}
    	attempts++;
    	usleep(5);
    }
    message.state = MESSAGE_ERROR;
	
	/*struct message_raw message_raw;
	memset(&message_raw.msg.data, '\0', MESSAGE_DATA_LENGTH);
	socklen_t addr_len = sizeof(struct sockaddr_in);
	
	while(1) {
		if (recvfrom(sfd, &message_raw, RAW_MESSAGE_LEN, 0, (struct sockaddr*) &svaddr, &addr_len) == -1)
			perror("recvfrom");
		else {
			message = message_raw.msg;
			printf("Packet is taken");
			if((sid = SERVICE_ID_NOT_DEFINED || message.sid == sid) && message.mid == mid)
				printf("%s", message.data);
			else
				perror("Wrong source");
		}
	}*/

	close(sfd);
	
	return 0;
}
