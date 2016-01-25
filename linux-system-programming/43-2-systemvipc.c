#include <stdio.h>
#include <unistd.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

int main(int argc, const char *argv[])
{
    char buff[4*1024*1024];
    pid_t pid;
    int msg_id;
    if (msg_id = msgget(IPC_PRIVATE, S_IRUSR|S_IWUSR) < 0) {
        perror("msgget ");
        exit(EXIT_FAILURE);
    }

    pid = fork();
    if (pid<0) {
        perror("fork ");
        exit(EXIT_FAILURE);
    } else if (pid>0) { // parent
        printf("Begin writing data...\n");
        int i = 2000;
        while(i--)
            msgsnd(msg_id, buff, sizeof(buff));
        printf("write data finish\n");
        close(pipefds[1]);          /* Reader will see EOF */
    } else { // child
        close(pipefds[1]);          /* Close unused write end */        
        long read_in_bytes = 0;
        int ret = 0;
        struct timeval tv_before,tv_after;  
        
        gettimeofday(&tv_before, NULL);
        while ((ret = read(pipefds[0], buff, sizeof(buff))) > 0) {
            read_in_bytes += ret;
        }
        gettimeofday(&tv_after, NULL);
         
        float total_time_in_second = (tv_after.tv_sec+tv_after.tv_usec/1000000) -
                                     (tv_before.tv_sec+tv_before.tv_usec/1000000);
        printf ("Read %ld bytes finish in %0.2f seconds\n", read_in_bytes ,total_time_in_second);
        float rate = read_in_bytes/(1024*1024*total_time_in_second);
        printf("Pipe rate in linux is %0.2f Mbps\n", rate);
        close(pipefds[0]);
    }

    return 0;
}
