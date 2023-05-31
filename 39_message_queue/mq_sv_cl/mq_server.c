/* Программа демонстрирует обмен сообщениями между клиентом и сервером
через очереди формата POSIX.
Версия 01. Дата релиза 09.04.2023 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <mqueue.h> 
#include <sys/stat.h> // Определяет константы для аргумента mode
#include <fcntl.h> // Определяет константы вида O_ 
#include "mq.h"

int main()
{
	/*if(mq_unlink(name) == -1) // Уничтожаем очередь
		perror("mq_unlink");*/
	mqd_t mqd;
	int flags = O_RDWR;
	mode_t perms = S_IRUSR | S_IWUSR; // ?
	struct mq_attr *attrp = NULL; // Очередь будет создана с параметрами по умолчанию
	
	mqd = mq_open("/test", O_RDWR);	
	if(mqd == (mqd_t)-1)
		perror("mq_open");
		
	struct request req;
	struct response resp;
	//mqd_t mqd;
	ssize_t numRead;
	
	unsigned int mqPrio = 1; // Приоритет сообщения
	unsigned int* mqPrio_ptr = &mqPrio;
	
	mqd = mq_open("/test", O_RDWR); // Открыли очередь - сервер читает и пишет в нее
	
	numRead = mq_receive(mqd, req.mqReq, MQ_SIZE, mqPrio_ptr); // Прочитали сообщ-е клиента (как узнать, зап-л он уже сообщ или нет)
	printf("\nСообщение клиента: %s\n", req.mqReq);
	
	if(strcmp(req.mqReq, "Hi!"))
	{
		strcpy(resp.mqResp, "Hello!"); // Записали наш ответ клиенту
		mq_send(mqd, resp.mqResp, MQ_SIZE, mqPrio); // Отправили
	}
	else
	{
		strcpy(resp.mqResp, "Wrong answer from server!"); // Записали наш ответ клиенту в случае ошибки
		mq_send(mqd, resp.mqResp, MQ_SIZE, mqPrio); // Отправили
		perror("Wrong answer from server!");
	}
	
	return 0;
}
