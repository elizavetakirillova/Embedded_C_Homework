#include <stdio.h>
#include <errno.h>
#include <mqueue.h>
#include "mq.h"

int main()
{
	if(mq_unlink("/test") == -1) // Уничтожаем очередь
		perror("mq_unlink");
	
	return 0;
}
