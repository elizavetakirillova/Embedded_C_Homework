#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <mqueue.h> 
#include <sys/stat.h>
#include <fcntl.h>
#include "mq.h"

int main()
{
	struct request req;
	struct response resp;
	mqd_t mqd;
	ssize_t numRead;
	
	unsigned int mqPrio = 1; // Приоритет сообщения
	unsigned int* mqPrio_ptr = &mqPrio;
	
	mqd = mq_open("/test", O_RDWR); // Открыли очередь - клиент читает и пишет в нее
	
	strcpy(req.mqReq, "Hello!"); // Записали наш запрос серверу
	mq_send(mqd, req.mqReq, MQ_SIZE, mqPrio); // Отправили
	
	numRead = mq_receive(mqd, resp.mqResp, MQ_SIZE, mqPrio_ptr); // Прочитали сообщ-е сервера
	printf("\nСообщение сервера: %s\n", resp.mqResp);
	
	return 0;
}
