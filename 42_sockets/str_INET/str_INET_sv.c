#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include "str_INET.h"

#define BACKLOG 5

int main(int argc, char* argv[])
{
	struct sockaddr_in addr;
	int sfd, cfd;
	ssize_t numRead;
	char buf[1024];
	
	sfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sfd == -1)
		perror("socket");
	
	/* Формируем адрес сервера, привязываем к нему сокет и делаем этот сокет слушающим */
	/*if (remove(SV_SOCK_PATH) == -1)
		perror("remove");*/
	
	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(3425); /* Преобразует узловой порядок расположения байтов положительного 
	короткого целого hostshort в сетевой порядок расположения байтов. */
	addr.sin_addr.s_addr = htonl(INADDR_ANY); /* Преобразует u_long из узла в порядок байтов сети 
	TCP/IP (который является байтом big-endian). */
	
	if (bind(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)) == -1)
		perror("bind");
	
	if (listen(sfd, BACKLOG) == -1)
		perror("listen");
		
	for (;;) { /* Последовательно обрабатываем клиентские соединения */
		/* Принимаем соединение. Оно будет назначено новому сокету,
		cfd'; слушающий сокет ('sfd') остается открытым и может
		принимать последующие соединения. */
		cfd = accept(sfd, NULL, NULL);
		if (cfd == -1)
			perror("accept");
		
		while(1)
		{
			numRead = recv(cfd, buf, 1024, 0);
			if(numRead == -1) break;
			buf[0] = 'W';
			if (send(cfd, buf, numRead, 0) == -1)
				perror("send");
		}
			
		if (close(cfd) == -1)
			perror("close");
	}
	
	/*if (close(sfd) == -1) // Почему не закрываем?
		perror("close");*/
	
	return 0;
}
