/* Демонстрация клиент-серверного обмена multycast */
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
    struct ip_mreqn mreqn;
    int socketfd;

    socketfd = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777); // Возвращает значение, приведенное к сетевому порядку следования байтов
    addr.sin_addr.s_addr = inet_addr(INADDR_ANY);

	if (bind(socketfd, (struct sockaddr*) &addr, sizeof(addr)) == -1)
		perror("bind");

    inet_aton("224.0.0.1", &(mreqn.imr_multiaddr));
    inet_aton(INADDR_ANY, &(mreqn.imr_address));
    //mreqn.imr_address = htons(INADDR_ANY); // почему так не получается
    mreqn.imr_ifindex = 0;

    setsockopt(socketfd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreqn, sizeof(mreqn));

    char data[BUF_SIZE];

    recvfrom(socketfd, (char *)data, BUF_SIZE, 0, NULL, NULL);
    
    printf("%s", data);

    return 0;
}
