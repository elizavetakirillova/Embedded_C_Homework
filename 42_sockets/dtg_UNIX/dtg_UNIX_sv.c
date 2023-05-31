#include <stdio.h>
#include <unistd.h>
#include "dtg_UNIX.h"

#define BACKLOG 5

int main(int argc, char* argv[])
{
	struct sockaddr_un svaddr, claddr;
	int sfd, cfd;
	ssize_t numBytes;
	socklen_t len;
	char buf[1024];
	
	sfd = socket(AF_UNIX, SOCK_DGRAM, 0);
	if (sfd == -1)
		perror("socket");
	
	/* Формируем общеизвестный адрес сервера и привязываем к нему серверный сокет */
	if (remove(SV_SOCK_PATH) == -1)
		perror("remove");
	
	memset(&svaddr, 0, sizeof(struct sockaddr_un));
	svaddr.sun_family = AF_UNIX;
	strncpy(svaddr.sun_path, SV_SOCK_PATH, sizeof(svaddr.sun_path) - 1);
	
	if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_un)) == -1)
		perror("bind");
	
	/* Принимаем сообщения */	
	for (;;) { 
		len = sizeof(struct sockaddr_un);
		numBytes = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &claddr, &len);	
		if (numBytes == -1)
			perror("recvfrom");
		
		buf[0] = 'W';
		
		if (sendto(sfd, buf, numBytes, 0, (struct sockaddr*) &claddr, len) == -1)
				perror("sendto");
	}
	
	return 0;
}
