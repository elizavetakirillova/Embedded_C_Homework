#include <stdio.h>
#include <unistd.h>
#include "dtg_UNIX.h"

int main(int argc, char* argv[])
{
	struct sockaddr_un svaddr, claddr;
	int sfd;
	char buf[] = "Hello!\n";
	char buf_rec[sizeof(buf)]; 
	//ssize_t numRead;
	
	/* Создаем клиентский сокет; привязываем его к уникальному пути
	(основанному на PID) */
	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd == -1)
		perror("perror");
	
	memset(&claddr, 0, sizeof(struct sockaddr_un));
	claddr.sun_family = AF_UNIX;
	snprintf(claddr.sun_path, sizeof(claddr.sun_path), "/tmp/ud_ucase_cl.%ld", (long) getpid());
	if (bind(sfd, (struct sockaddr*) &claddr, sizeof(struct sockaddr_un)) == -1)
		perror("bind");
	
	/* Формируем адрес сервера */
	memset(&svaddr, 0, sizeof(struct sockaddr_un));
	svaddr.sun_family = AF_UNIX;
	strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) -1);
	
	/* Копируем в сокет содержимое буфера запроса */
	if (sendto(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &svaddr, sizeof(struct sockaddr_un)) == -1)
		perror("sendto");
	
	if (recvfrom(sfd, buf_rec, sizeof(buf_rec), 0, NULL, NULL) == -1)
		perror("recvfrom");
	
	printf("%s", buf_rec);
	remove(claddr.sun_path);
	
	return 0;
}
