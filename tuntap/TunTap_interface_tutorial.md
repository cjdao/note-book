## Tun/Tap interface tutorial
### 本文主要参考：  
[Tun/Tap interface tutorial](http://backreference.org/2010/03/26/tuntap-interface-tutorial/)

### 它们是什么以及如何工作的
* tun/tap 接口是一种纯软件上的接口，它仅存在于内核空间。它不与任何物理网络设备关联。

* 它的唯一作用，使得用户空间的程序能够直接获取ip层或以太网层的网络流量(貌似raw socket也有这样的功能，它们的区别是什么呢？)。  

* 用户空间的程序需要从一个tun/tap 接口获取网络流量数据前，必须先挂载该接口。挂载到某个tun/tap 接口的用户程序会得到一个文件描述符，用户程序可以据此进行read/write等操作.  

* 当内核网络栈向tun/tap接口发生数据时，挂载了该接口的用户空间的程序就可以通过read操作获取到网络数据。当用户程序向tun/tap接口write数据时，内核的网络协议栈就会收到该数据。  

* tap 接口和 tun 接口的区别在于，tap接口传输的是以太网帧，而tun接口传输的是ip数据包。

* tap/tun接口能以两种模式存在，一种是临时的(transient)另一种是持久化(persistent)的. 临时模式是指，tap/tun接口被应用程序创建出来后，当应用程序退出后，tap/tun接口就会自动被销毁(一般情况下，当tap/tun接口被应用程序创建出来后，它总会被关联到某个文件句柄上，当程序退出，该文件句柄会被自动关闭，因此tap/tun接口也就被销毁了)；持久化模式是指，创建tap/tun接口的应用程序退出后，接口继续存在。

* 一旦一个tap/tun接口被创建出来后，那么它就能够像普通的网络接口那样，能被指定IP，能被用于路由规则，能被用于防火墙规则，网络分析工具能够获取到该接口的数据包。

### 如何创建tap/tun接口

#### 命令行层面的创建
iproute2工具箱里有相关的命令支持tap/tun的创建, 
```
ip tuntap add dev ${interface name} mode tap/tun
```

### 代码层面的创建
1. 权限问题
只有root或者拥有CAP_NET_ADMIN能力的程序才能创建tap/tun接口
2. 创建方法
 1. 以读写的方式打开文件**/dev/net/tun**, 获得一个文件描述符
 2. 在步骤1获得的文件描述符上执行一个ioctl操作，参数有两个，一个是TUNSETIFF，另一个是一个数据结构指针，包含描述该接口的参数(如接口名，是tun还是tap接口等)

> Note:完成了上面两个步骤，创该tap/tun接口的应用程序，就可以通过步骤1中的文件描述符来读写网络数据包了。但是这样创建出来的tap/tun接口是临时性的，当应用程序退出后，接口也就会被自动销毁。

> Note:关于接口名，用户可以不指定，而由内核自动分配。

内核源码树中Documentation/networking/tuntap.txt文件，描述了如何通过代码创建一个tap/tun接口，我们以此为基础封装了一个函数：
```cpp
#include <linux /if.h>
#include <linux /if_tun.h>

// 函数名：tun_alloc
// 功能：创建
// 参数：
//    char *dev  输入输出参数 用于指定及存放创建的接口名
//    int flags 输入参数 指定创建的是tap还是tun(值：FF_TUN or IFF_TAP, plus maybe IFF_NO_PI)
// 返回值： int  与tun/tap接口关联的文件句柄   
int tun_alloc(char *dev, int flags) {

  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  /* Arguments taken by the function:
   *
   * char *dev: the name of an interface (or '\0'). MUST have enough
   *   space to hold the interface name if '\0' is passed
   * int flags: interface flags (eg, IFF_TUN etc.)
   */

   /* open the clone device */
   if( (fd = open(clonedev, O_RDWR)) < 0 ) {
     return fd;
   }

   /* preparation of the struct ifr, of type "struct ifreq" */
   memset(&ifr, 0, sizeof(ifr));

   ifr.ifr_flags = flags;   /* IFF_TUN or IFF_TAP, plus maybe IFF_NO_PI */

   if (*dev) {
     /* if a device name was specified, put it in the structure; otherwise,
      * the kernel will try to allocate the "next" device of the
      * specified type */
     strncpy(ifr.ifr_name, dev, IFNAMSIZ);
   }

   /* try to create the device */
   if( (err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0 ) {
     close(fd);
     return err;
   }

  /* if the operation was successful, write back the name of the
   * interface to the variable "dev", so the caller can know
   * it. Note that the caller MUST reserve space in *dev (see calling
   * code below) */
  strcpy(dev, ifr.ifr_name);

  /* this is the special file descriptor that the caller will use to talk
   * with the virtual interface */
  return fd;
}
``` 
我们可以这么使用该函数：
```cpp
char tun_name[IFNAMSIZ];
char tap_name[IFNAMSIZ];
char *a_name;
int tunfd,tapfd;

//...

strcpy(tun_name, "tun1");
tunfd = tun_alloc(tun_name, IFF_TUN);  /* tun interface */

strcpy(tap_name, "tap44");
tapfd = tun_alloc(tap_name, IFF_TAP);  /* tap interface */

a_name = malloc(IFNAMSIZ);
a_name[0]='\0';
tapfd = tun_alloc(a_name, IFF_TAP);    /* let the kernel pick a name */
```
