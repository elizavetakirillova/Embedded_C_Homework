#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#define BUF_SIZE 10

char buf[BUF_SIZE];

int main(void)
{
    ssize_t l;
    int pfd1[2],pfd2[2];

    if(pipe(pfd1)){
       perror("pipe1");
       return 1;
    }
    if(pipe(pfd2)){
       perror("pipe2");
       return 2;
    }

    switch(fork()){
       case -1:
           perror("fork");
           return 3;
       case 0:/*Child*/
           close(pfd1[0]);
           close(pfd2[1]);
           strcpy(buf,"Hi!");
           l = strlen(buf);

           write(pfd1[1], buf, BUF_SIZE);

           read(pfd2[0], buf, BUF_SIZE);

           printf("Child from the father: '%s'\n", buf);
           return 0;
       default:/*The father*/
           close(pfd1[1]);
           close(pfd2[0]);
           break;
    }

    read(pfd1[0], buf, BUF_SIZE);
    printf("Father from child: '%s'\n", buf);

    strcpy(buf,"Hello!");
    l=strlen(buf);

    write(pfd2[1], buf, BUF_SIZE);

    wait(NULL);
    return 0;
}
