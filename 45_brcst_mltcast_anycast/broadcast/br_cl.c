#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <string.h>
#define BUF_SIZE 7

int main()
{
    struct sockaddr_in addr;
    int socketfd;
    
    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777); // Возвращает значение, приведенное к сетевому порядку следования байтов
    inet_aton("255.255.255.255", &addr.sin_addr.s_addr);
    // addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);

	if (bind(socketfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
		perror("bind");

    char data[BUF_SIZE];

    recvfrom(socketfd, (char *)data, BUF_SIZE, 0, NULL, NULL);
    
    printf("%s", data);

    return 0;
}
