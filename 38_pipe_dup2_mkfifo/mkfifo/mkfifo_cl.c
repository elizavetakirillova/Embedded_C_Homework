#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include "mkfifo.h"

int main(int argc, char **argv)
{
	int serverFd, clientFd;
	struct request req;
	struct response resp;
	char clientMsg[] = "Hello!"; // Сообщение клиента

	/* открыли канал на запись */	
	clientFd = open("fifo", O_WRONLY | O_NONBLOCK);
	req.pid = getpid();
	strcpy(req.seqLen, clientMsg);
	write(clientFd, &req, sizeof(struct request));
	close(clientFd);

	/* открыли канал на чтение */	
	serverFd = open ("fifo", O_RDONLY | O_NONBLOCK);
	read(serverFd, &resp, sizeof(struct response));
	printf("Client %d got message \"%s\" from server!\n", getpid(), resp.seqNum);
	close(serverFd);

	return 0;
}
