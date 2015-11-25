#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define MAXBUFFSIZE  (200)
#define SERVERPORT  (1033)

int main(int argc, char *argv[])
{
    int sockfd, n;
    char recvline[MAXBUFFSIZE];
    struct sockaddr_in servaddr;
    int ret;
    
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("Open a socket: ");
        return -1;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVERPORT);
    
    if (1!=inet_pton(AF_INET, argv[1], &servaddr.sin_addr)) {
        printf ("server address error.\n"); 
        return -1;
    }

    
    ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if (ret < 0) {
        perror("connect ");
        return -1;
    }

    while ((n=read(sockfd, recvline, MAXBUFFSIZE)) > 0) {
        if (fputs(recvline, stdout) == EOF)  {
            printf("fputs error.\n");
        } 
    }
    
    return 0;
}
