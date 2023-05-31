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
	mqd_t mqd;
	struct mq_attr *attrp = NULL; // Очередь будет создана с параметрами по умолчанию
	
	mqd = mq_open("/test", O_RDWR|O_CREAT, 0666, NULL);	
	if(mqd == (mqd_t)-1)
		perror("mq_open");
	
	return 0;
}
