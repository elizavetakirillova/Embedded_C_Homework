#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "str_INET.h"

int main(int argc, char* argv[])
{
	struct sockaddr_in addr;
	int sfd;
	char buf[] = "Hello!\n";
	char buf_rec[sizeof(buf)]; 
	ssize_t numRead;
	
	sfd = socket(AF_INET, SOCK_STREAM, 0); /* Создаем клиентский сокет */
	if (sfd == -1)
		perror("perror");
	
	/* Формируем адрес сервера и выполняем соединение */
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(3425);
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	
	if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
		perror;
		
	/* Копируем в сокет содержимое буфера запроса */
	if (send(sfd, buf, sizeof(buf), 0) == -1)
		perror("failed read");
	
	if (recv(sfd, buf_rec, sizeof(buf), 0) == -1)
		perror("failed read");
	
	printf("%s", buf_rec);
	close(sfd);
	
	return 0;
}
