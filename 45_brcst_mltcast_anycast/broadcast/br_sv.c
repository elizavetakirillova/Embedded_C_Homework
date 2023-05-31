/* Демонстрация клиент-серверного обмена broadcast */
#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <string.h>

int main()
{
    struct sockaddr_in addr;
    int socketfd;
	char* buf = "Hello";
	
    socketfd = socket(AF_INET,SOCK_DGRAM,0);
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777); // Любой порт можем придумать - нет понятия широковещательного порта
    inet_aton("255.255.255.255", &addr.sin_addr.s_addr); // Широковещательный адрес
    // addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
    
    int flag = 1; // Включить опцию SO_BROADCAST
    setsockopt(socketfd, SOL_SOCKET, SO_BROADCAST, &flag, sizeof(flag));

    while(1) {
        if(sendto(socketfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0) 
            perror("something went wrong..");
    }

    return 0;
}
