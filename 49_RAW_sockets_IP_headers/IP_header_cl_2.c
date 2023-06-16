/* Демонстрация IP-заголовка для обмена пакетами по сети
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

// структура IP-заголовка
struct ip_header  {
#if __BYTE_ORDER == __LITTLE_ENDIAN
    unsigned int ihl:4;
    unsigned int version:4;
#elif __BYTE_ORDER == __BIG_ENDIAN
    unsigned int version:4;
    unsigned int ihl:4;
#else
# error        "Please fix <bits/endian.h>"
#endif
    u_int8_t tos;
    u_int16_t tot_len;
    u_int16_t id;
    u_int16_t frag_off;
    u_int8_t ttl;
    u_int8_t protocol;
    u_int16_t check;
    u_int32_t saddr;
    u_int32_t daddr;
    /*The options start here. */
};

// структура сообщения
struct message {
	unsigned short sid; // id привязанного сервиса
	unsigned short mid; // id сообщения
	unsigned short type; // тип сообщения (0 запрос подключения, 1 запрос отключения, 2 передача данных)
	unsigned short state; // состояние сообщения (0 подготовлено, 1 получено, 2 отправлено, 3 ошибка)
	char data[MESSAGE_DATA_LENGTH]; // данные сообщения
};

// структура IP-сообщения
struct ip_message {
	struct ip_header iph;
	struct udp_header udph;
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
	char source_ip[32];
	strcpy(source_ip , "127.0.0.1");
	
	/* Создаем клиентский сокет; привязываем его к уникальному пути
	(основанному на PID) */
	sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (sfd == -1)
		perror("perror");

	int flag = 1; // Включить опцию IP_HDRINCL
    setsockopt(sfd, IPPROTO_IP, IP_HDRINCL, &flag, sizeof(flag));

	/* IP-заголовок */
	struct ip_message ip_message; 
	//struct udphdr* udph = (struct udphdr*) udp_message.msg.data;

	/* Формируем адрес сервера */
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(DEFAULT_PORT_SERVER);
	svaddr.sin_addr.s_addr = htonl(DEFAULT_SERVER_IP); // inet_addr("127.0.0.1"); // адрес сервера

	//IP header
	//struct iphdr *iph = (struct iphdr *) udp_message.msg.data;
	ip_message.iph.ihl = 5;
	ip_message.iph.version = 4;
	ip_message.iph.tos = 0;
	ip_message.iph.tot_len = RAW_MESSAGE_LEN;
	ip_message.iph.id = htonl (54321);	//Id of this packet
	ip_message.iph.frag_off = 0;
	ip_message.iph.ttl = 255;
	ip_message.iph.protocol = IPPROTO_UDP;
	ip_message.iph.check = 0;
	ip_message.iph.saddr = inet_addr (source_ip);	//Spoof the source ip address
	ip_message.iph.daddr = htons(DEFAULT_PORT_SERVER);

	// формируем UPD заголовок сообщения
	ip_message.udph.source_port=htons(DEFAULT_PORT_CLIENT); // Порт отправителя
	ip_message.udph.dest_port=htons(DEFAULT_PORT_SERVER); // Порт получателя
	ip_message.udph.packet_len=htons(UDP_MESSAGE_LENGTH); // Длина пакета
	ip_message.udph.check_sum=0; // Контрольная сумма (не проверяем)

	/* Подготавливаем данные к отправке */
	memset(&ip_message.msg.data, '\0', MESSAGE_DATA_LENGTH);
	strcpy(ip_message.msg.data, "Hello!");
	ip_message.msg.type = type;
	ip_message.msg.sid = sid;
	ip_message.msg.mid = mid;
	ip_message.msg.state = MESSAGE_PREPARED;
	
	if (sendto(sfd, &ip_message, RAW_MESSAGE_LEN, 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("sendto");
	else
		printf("Packet Send. Length: %d \n", ip_message.iph.tot_len);

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
    
	close(sfd);
	
	return 0;
}
