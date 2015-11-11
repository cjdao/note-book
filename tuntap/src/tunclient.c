  /* tunclient.c */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <linux/if.h>
#include <linux/if_tun.h>

#include "./tun_alloc.h"
static char buffer[4096];

int main(int argc, char *argv[])
{
	char tun_name[IFNAMSIZ];
    int tun_fd=-1;
    int nread = 0;	
    
    if (argc!=3) {
        printf("usage: %s intefacename(etc:tun0) mode(tun/tap)\n", argv[0]);
        exit(1);
    }
    
    int flag = 0;
    if (strncmp("tun",argv[2],3) == 0) {
        flag = IFF_TUN | IFF_NO_PI;
    } else if (strncmp("tap", argv[2], 3)!=0) {
        flag = IFF_TAP | IFF_NO_PI;
    } else {
        printf("usage: %s intefacename(etc:tun0) mode(tun/tap)\n", argv[0]);
        exit(1);
    }
    
	/* Connect to the device */
	strncpy(tun_name, argv[1], IFNAMSIZ);
	tun_fd = tun_alloc(tun_name, flag);  /* tun interface */
	
	if(tun_fd < 0){
	  perror("Allocating interface");
	  exit(1);
	}
	
	/* Now read data coming from the kernel */
	while(1) {
	  /* Note that "buffer" should be at least the MTU size of the interface, eg 1500 bytes */
	  nread = read(tun_fd,buffer,sizeof(buffer));
	  if(nread < 0) {
	    perror("Reading from interface");
	    close(tun_fd);
	    exit(1);
	  }
	
	  /* Do whatever with the data */
	  printf("Read %d bytes from device %s\n", nread, tun_name);
	}
}
