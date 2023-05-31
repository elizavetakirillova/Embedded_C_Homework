#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "dtg_INET.h"

int main(int argc, char* argv[])
{
	struct sockaddr_in svaddr, claddr;
	int sfd;
	char buf[] = "Hello!\n";
	char buf_rec[sizeof(buf)]; 
	//ssize_t numRead;
	
	/* Создаем клиентский сокет; привязываем его к уникальному пути
	(основанному на PID) */
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd == -1)
		perror("perror");
	
	memset(&claddr, 0, sizeof(struct sockaddr_in));
	claddr.sin_family = AF_INET;
	claddr.sin_port = htons(3425);
	claddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	
	/*if (bind(sfd, (struct sockaddr*) &claddr, sizeof(struct sockaddr_in)) == -1)
		perror("bind");*/
	
	/* Формируем адрес сервера */
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(3425);
	svaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	
	/* Копируем в сокет содержимое буфера запроса */
	if (sendto(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("sendto");
	
	if (recvfrom(sfd, buf_rec, sizeof(buf_rec), 0, NULL, NULL) == -1)
		perror("recvfrom");
	
	printf("%s", buf_rec);
	//remove(claddr.sin_path);
	
	return 0;
}
