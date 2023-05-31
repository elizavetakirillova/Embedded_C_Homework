/* Демонстрация UDP-заголовка для обмена пакетами по сети
клиента с сервером */
#include <stdio.h>
#include <string.h> //memset
#include <stdlib.h> //for exit(0);
#include <unistd.h>
#include <errno.h> //For errno - the error number
#include <netinet/in.h>
#include <netinet/udp.h>	//Provides declarations for udp header
#include <netinet/ip.h>	//Provides declarations for ip header
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>

int main(int argc, char* argv[])
{
	struct sockaddr_in svaddr; //, claddr;
	int sfd;
	char buf[] = "12345678Hello!"; // Размер буфера с учетом UDP заголовка - отправляемые данные
	char buf_rec[sizeof(buf)/sizeof(buf[0]) + 28]; // Размер буфера с учетом UDP и IP заголовков - принимаемые данные

	/* Создаем клиентский сокет; привязываем его к уникальному пути
	(основанному на PID) */
	sfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	if (sfd == -1)
		perror("perror");

	/* UDP-заголовок */
	struct udphdr* udph = (struct udphdr*) buf;

	/* Формируем адрес сервера */
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(3425);
	svaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK); // inet_addr("127.0.0.1"); // адрес сервера
	
	/* UDP-заголовок */
	udph->source = htons (6666); // вымышленный порт
	udph->dest = htons (3425); // порт сервера
	udph->len = htons(8 + strlen(buf));	// размер UDP заголовка
	udph->check = 0; // ставим 0

	if (sendto(sfd, buf, sizeof(udph->len), 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("sendto");
	else
		printf("Packet Send. Length: %d \n", udph->len);

	while(1) {
		if (recvfrom(sfd, buf_rec, sizeof(udph), 0, NULL, NULL) == -1)
			perror("recvfrom");
		else {
			printf("Packet is taken");
			int port = htonl(*(buf_rec + 20*sizeof(char)));
			if (port == udph->source)
				printf("%s", buf_rec + 28*sizeof(char));
			else
				perror("Wrong source");
		}
	}
	
	//close(svaddr); нужно ли закрывать сокет?
	
	return 0;
}
