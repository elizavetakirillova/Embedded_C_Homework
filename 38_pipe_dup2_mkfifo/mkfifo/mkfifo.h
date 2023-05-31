#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#define BUFSIZE 20

struct request { // Запрос (клиент --> сервер)
    pid_t pid; // PID клиента
    char seqLen[BUFSIZE];
    // int seqLen; // Длина запрашиваемой последовательности
};

struct response { // Ответ (сервер --> клиент)
    char seqNum[BUFSIZE];
};
