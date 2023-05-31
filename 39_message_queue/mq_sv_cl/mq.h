#include <mqueue.h> 
#define MQ_SIZE 10 // Размер очереди
#define NUM 10
	
struct request { // Запрос (клиент --> сервер)
    char mqReq[NUM];
};

struct response { // Ответ (сервер --> клиент)
    char mqResp[NUM];
};
