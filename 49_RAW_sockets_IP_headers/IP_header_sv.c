#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>

#define DEFAULT_SERVER_IP INADDR_LOOPBACK // IP адрес сервера по умолчанию (127.0.0.1)
#define DEFAULT_PORT_SERVER 8000 // номер порта сервера по умолчанию (8000)
#define DEFAULT_PORT_CLIENT 8001 // номер порта клиента по умолчанию (8001)

// параметры UDP сообщения
#define UDP_HEADER_SIZE 8 // стандартный размер заголовка UPD-пакета в байтах
#define MESSAGE_DATA_LENGTH 20 // размер данных сообщения в байтах (sizeof char = 1 байт)
#define MESSAGE_FULL_LENGTH MESSAGE_DATA_LENGTH + 2 // полная длина сообщения в байтах (sizeof unsigned short = 2 байта) /* Почему тут 8? */
#define UDP_MESSAGE_LENGTH UDP_HEADER_SIZE + MESSAGE_FULL_LENGTH // полная длина UDP-сообщения пересылаемого по сети
#define IP_PLUS_UDP_SIZE 20 + 8
#define RAW_MESSAGE_LEN IP_PLUS_UDP_SIZE + MESSAGE_FULL_LENGTH

int main(int argc, char* argv[])
{
	struct sockaddr_in svaddr, claddr;
	int sfd; //, cfd;
	ssize_t numBytes;
	socklen_t len;
	char buf[MESSAGE_DATA_LENGTH];
	
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd == -1)
		perror("socket");
	
	/* Формируем общеизвестный адрес сервера и привязываем к нему серверный сокет */
	/*if (remove(SV_SOCK_PATH) == -1)
		perror("remove");*/
	
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(DEFAULT_PORT_SERVER);
	svaddr.sin_addr.s_addr = htonl(DEFAULT_SERVER_IP);
	
	if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("bind");
	
	/* Принимаем сообщения */	
	for (;;) { 
		len = sizeof(struct sockaddr_in);
		numBytes = recvfrom(sfd, buf, MESSAGE_FULL_LENGTH, 0, (struct sockaddr*) &claddr, &len);	
		if (numBytes == -1)
			perror("recvfrom");
		
		buf[0] = 'W';
		
		if (sendto(sfd, buf, MESSAGE_FULL_LENGTH, 0, (struct sockaddr*) &claddr, len) == -1)
				perror("sendto");
	}
	
	return 0;
}
