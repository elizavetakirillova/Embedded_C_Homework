#include <stdio.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <ctype.h>
#include <string.h>
#include <netinet/ip.h>

int main()
{
    struct sockaddr_in addr;
    int socketfd;
    char* buf = "Hello";
    char* group = "224.0.0.1";
    
    socketfd = socket(AF_INET, SOCK_DGRAM,0);
    memset(&addr, 0, sizeof(struct sockaddr_in));

    addr.sin_family = AF_INET;
    addr.sin_port = htons(7777);
    inet_aton("224.0.0.1", &addr.sin_addr.s_addr);

    while(1) {
        if(sendto(socketfd, buf, strlen(buf), 0, (struct sockaddr *)&addr, sizeof(addr)) <= 0) 
            perror("something went wrong..");
    }
    
    return 0;
}
