#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAXBUFSIZE (200)
#define LISTENQUE (10)
#define LISTENPORT (1033)

int main(int argc, char *argv[])
{
    int listenfd, connfd;
    struct sockaddr_in servaddr;
    char buff[MAXBUFSIZE];
    time_t ticks;
    int ret;
    
    listenfd = socket(PF_INET, SOCK_STREAM, 0);
    if (listenfd < 0) {
        perror("Open a socket: ");
        return -1;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(LISTENPORT);
    
    ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)); 
    if (ret < 0) {
        perror("Bind a socket: ");
        return -1;
    }
     
    ret = listen(listenfd, LISTENQUE);
    if (ret < 0) {
        perror("listen a socket: ");
        return -1;
    }
    
    printf("\n\nTime server is running on port %d.\n", LISTENPORT);
    
    while(1) {
        connfd = accept(listenfd, NULL, NULL);
#if 0
        if (connfd < 0) {
            perror("Accept: ");
            continue;
        }
        printf("Time server get a request from client\n");
        ticks = time(NULL);
        snprintf (buff, sizeof(buff), "%.24s\n", ctime(&ticks));
        write(connfd, buff, strlen(buff));
        close(connfd);
#endif
        if (connfd>=0)
            close(connfd);
    }

    return 0;
}
