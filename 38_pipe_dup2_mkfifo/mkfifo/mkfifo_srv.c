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
	int serverFd, clientFd, dummyFd;
	struct request req;
	struct response resp;
	char serverMsg[] = "Hi!"; // Сообщение сервера
	
	/* создали специальный файл FIFO с открытыми для всех правами доступа на чтение и запись */
	mkfifo("fifo", S_IFIFO | 0666);
	
	/* открыли канал на чтение */
	serverFd = open ("fifo", O_RDONLY);
	read (serverFd, &req, sizeof(struct request));
    printf ("Server %d got message \"%s\" from cl %d !\n", getpid(), req.seqLen, req.pid);
	close(serverFd);
		
	/* открыли канал на запись */	
	clientFd = open ("fifo", O_WRONLY);
	strcpy(resp.seqNum, serverMsg);
	write(clientFd, &resp, sizeof(struct response));
	close(clientFd);
		
	/* уничтожили именованный канал */
	unlink ("fifo");
	
	return 0;
}
