#include <stdio.h>
#include <unistd.h>
#include "str_UNIX.h"

int main(int argc, char* argv[])
{
	struct sockaddr_un addr;
	int sfd;
	char buf[] = "Hello!\n";
	char buf_rec[sizeof(buf)]; 
	ssize_t numRead;
	
	sfd = socket(AF_UNIX, SOCK_STREAM, 0); /* Создаем клиентский сокет */
	if (sfd == -1)
		perror("perror");
	
	/* Формируем адрес сервера и выполняем соединение */
	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, SV_SOCK_PATH, sizeof(addr.sun_path) -1);
	
	if (connect(sfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_un)) == -1)
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
