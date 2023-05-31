#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <ctype.h>

int main(int argc, char* argv[])
{
	struct sockaddr_in svaddr, claddr;
	int sfd; //, cfd;
	ssize_t numBytes;
	socklen_t len;
	char buf[1024];
	
	sfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sfd == -1)
		perror("socket");
	
	/* Формируем общеизвестный адрес сервера и привязываем к нему серверный сокет */
	/*if (remove(SV_SOCK_PATH) == -1)
		perror("remove");*/
	
	memset(&svaddr, 0, sizeof(struct sockaddr_in));
	svaddr.sin_family = AF_INET;
	svaddr.sin_port = htons(3425);
	svaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(sfd, (struct sockaddr *) &svaddr, sizeof(struct sockaddr_in)) == -1)
		perror("bind");
	
	/* Принимаем сообщения */	
	for (;;) { 
		len = sizeof(struct sockaddr_in);
		numBytes = recvfrom(sfd, buf, sizeof(buf), 0, (struct sockaddr*) &claddr, &len);	
		if (numBytes == -1)
			perror("recvfrom");
		
		buf[0] = 'W';
		
		if (sendto(sfd, buf, numBytes, 0, (struct sockaddr*) &claddr, len) == -1)
				perror("sendto");
	}
	
	return 0;
}
