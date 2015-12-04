## Tun/Tap interface tutorial

### 它们是什么以及如何工作的
* tun/tap 接口是一种纯软件上的接口，它仅存在于内核空间。它不与任何物理网络设备关联。

* 它的唯一作用，使得用户空间的程序能够直接获取ip层或以太网层的网络流量(貌似raw socket也有这样的功能，它们的区别是什么呢？)。  

* 用户空间的程序需要从一个tun/tap 接口获取网络流量数据前，必须先连接该接口。连接到某个tun/tap 接口的用户程序会得到一个文件描述符，用户程序可以据此进行read/write等操作.  

* 当内核网络栈向tun/tap接口发送数据时，连接了该接口的用户空间的程序就可以通过read操作获取到网络数据。当用户程序向tun/tap接口write数据时，内核的网络协议栈就会收到该数据。  

* tap 接口和 tun 接口的区别在于，tap接口传输的是以太网帧，而tun接口传输的是ip数据包。

* tap/tun接口能以两种模式存在，一种是临时的(transient)另一种是持久化(persistent)的. 临时模式是指，tap/tun接口被应用程序创建出来后，当应用程序退出后，tap/tun接口就会自动被销毁(一般情况下，当tap/tun接口被应用程序创建出来后，它总会被关联到某个文件句柄上，当程序退出，该文件句柄会被自动关闭，因此tap/tun接口也就被销毁了)；持久化模式是指，创建tap/tun接口的应用程序退出后，接口继续存在。

* 一旦一个tap/tun接口被创建出来后，那么它就能够像普通的网络接口那样，能被指定IP，能被用于路由规则，能被用于防火墙规则，网络分析工具能够获取到该接口的数据包。

### 如何创建tap/tun接口

#### 命令行层面的创建
* iproute2工具箱里有相关的命令支持tap/tun的创建(持久性的接口) 
```
ip tuntap add dev ${interface name} mode tap/tun
```
* 列出系统中所有的tap/tun接口
```
ip tuntap list 
```

#### 代码层面的创建
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

### tun/tap接口的持久化，及所有者 
* tun/tap接口的持久化
```cpp
// 接口持久化
ioctl(tap_fd, TUNSETPERSIST, 1);
// 接口临时化
ioctl(tap_fd, TUNSETPERSIST, 0);
```

* tun/tap接口的所有者，及所属组
```cpp
// 设置所有者
ioctl(tap_fd, TUNSETOWNER, owner);
// 设置所属组
ioctl(tap_fd, TUNSETGROUP, group);
```
* tunctl 工具所实现的上述功能的代码片段
```cpp
...
  /* "delete" is set if the user wants to delete (ie, make nonpersistent)
     an existing interface; otherwise, the user is creating a new
     interface */
  if(delete) {
    /* remove persistent status */
    if(ioctl(tap_fd, TUNSETPERSIST, 0) < 0){
      perror("disabling TUNSETPERSIST");
      exit(1);
    }
    printf("Set '%s' nonpersistent\n", ifr.ifr_name);
  }
  else {
    /* emulate behaviour prior to TUNSETGROUP */
    if(owner == -1 && group == -1) {
      owner = geteuid();
    }

    if(owner != -1) {
      if(ioctl(tap_fd, TUNSETOWNER, owner) < 0){
        perror("TUNSETOWNER");
        exit(1);
      }
    }
    if(group != -1) {
      if(ioctl(tap_fd, TUNSETGROUP, group) < 0){
        perror("TUNSETGROUP");
        exit(1);
      }
    }

    if(ioctl(tap_fd, TUNSETPERSIST, 1) < 0){
      perror("enabling TUNSETPERSIST");
      exit(1);
    }

    if(brief)
      printf("%s\n", ifr.ifr_name);
    else {
      printf("Set '%s' persistent and owned by", ifr.ifr_name);
      if(owner != -1)
          printf(" uid %d", owner);
      if(group != -1)
          printf(" gid %d", group);
      printf("\n");
    }
  }
  ...
```

### 应用程序连接tun/tap接口
连接一个tun/tap接口与创建一个tun/tap接口的代码是一模一样的，即tun_alloc()函数的实现。当然，欲连接tun/tap接口的用户必须满足以下条件:  
* 被连接的tun/tap接口已经存在，并且用户是该接口的所有者
* 用户必须拥有/dev/net/tun文件的读写权限
* 提供给tun_alloc()的flag参数必须与创建时的匹配(如创建时指定的是IFF_TUN，则连接时必须也是IFF_TUN)

当用户调用ioctl(TUNSETIFF)时，内核是如何区分用户是希望创建一个tun/tap接口还是连接一个tun/tap接口的呢？
* 如果没有提供接口名称，或者名称指定的接口不存在，则内核认为，用户程序希望创建一个新的tun/tap接口。这只有root用户能成功执行.
* 名称指定的接口存在，则内核认为用户希望连接一个之前已经创建了的tun/tap接口

> Note: 内核源文件drivers/net/tun.c里的tun_attach(), tun_net_init(), tun_set_iff(), tun_chr_ioctl()函数提供大部分的tun/tap接口的功能实现

> Note: 非root权限的用户是无法配置tun/tap接口的(如指定IP)

### 练练手
创建一个tun接口，并设置IP:  
```bash
# sudo ip tuntap add dev tun2 mode tun
# sudo ip tuntap add dev tap2 mode tap
# sudo ip l set dev tun2 up
# sudo ip l set dev tap2 up
# sudo ip a add dev tun2 10.1.1.1/24
# sudo ip a add dev tap2 10.0.0.1/24
# sudo ip route show 
...
10.0.0.0/24 dev tap2  proto kernel  scope link  src 10.0.0.10 
10.1.1.0/24 dev tun2  proto kernel  scope link  src 10.1.1.1 
...
```
ping 一下:  
```bash
# ping 10.1.1.1
PING 10.1.1.1 (10.1.1.1) 56(84) bytes of data.
64 bytes from 10.1.1.1: icmp_seq=1 ttl=64 time=0.106 ms
64 bytes from 10.1.1.1: icmp_seq=2 ttl=64 time=0.070 ms

# ping 10.0.0.1
PING 10.0.0.1 (10.0.0.1) 56(84) bytes of data.
64 bytes from 10.0.0.1: icmp_seq=1 ttl=64 time=0.108 ms
64 bytes from 10.0.0.1: icmp_seq=2 ttl=64 time=0.066 ms
```
可以看到是通的  

tcpdump 分析端口数据:  
```bash
# sudo tcpdump -i tun2 -n icmp
# sudo tcpdump -i tap2 -n icmp
```
发现上面两个接口都没有数据流  
这是正确的但是也是容易使人混乱的地方，究其原因，是当我们向一个系统的接口IP执行Ping命令时，内核能够正确的识别出，数据包不需要通过该接口发生出去，内核协议栈自身能够回复这些icmp请求。所以，我们在接口出看不到任何数据包。  

好,那我们ping一下跟tun/tap接口ip同一个网段的IP, 理论上，这个时候，我们应该能通过tcpdump抓到数据包了吧。但是，令人沮丧的是，我们还是没有抓到任何数据包。
```bash
# ping 10.0.0.2
# ping 10.1.1.2
```
到这里我们还是无法从tun/tap接口获取到数据包，通过实验，我们发现当tun/tap接口没有被挂载的时候，是抓取不到数据包的。原因嘛，还不知道：P  

那我们下面就下来实现一个挂载到tun/tap接口，并抓取数据包的程序([tunclient.c](https://github.com/cjdao/note-book/blob/master/tuntap/src/tunclient.c))  
然后，我们通过该程序挂载tun/tap接口  
```bash
# sudo ./tunclient tun2 tun
# sudo ./tunclient tap2 tap
```
于是乎，我们看到了这个
```bash
#  sudo tcpdump -i tun2 -n 
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on tun2, link-type RAW (Raw IP), capture size 65535 bytes
16:20:16.876830 IP 10.1.1.1 > 10.1.1.2: ICMP echo request, id 10496, seq 1, length 64
16:20:17.876345 IP 10.1.1.1 > 10.1.1.2: ICMP echo request, id 10496, seq 2, length 64

# sudo tcpdump -i tap2 -n 
tcpdump: verbose output suppressed, use -v or -vv for full protocol decode
listening on tap2, link-type EN10MB (Ethernet), capture size 65535 bytes
16:17:50.336292 ARP, Request who-has 10.0.0.2 tell 10.0.0.1, length 28
16:17:51.336276 ARP, Request who-has 10.0.0.2 tell 10.0.0.1, length 28
```
注意，我们在tun接口看到的是icmp包，而在tap接口看到的是arp包

同时，我们的tunclient程序也接受到了数据包，这意味着什么呢，看下面，懒得翻译了：
>Now what can we do with this data? Well, we could for example emulate the behavior of the target of the traffic we're reading; again, to keep things simple, let's stick with the ping example. We could analyze the received packet, extract the information needed to reply from the IP header, ICMP header and payload, build an IP packet containing an appropriate ICMP echo reply message, and send it back (ie, write it into the descriptor associated with the tun/tap device). This way the originator of the ping will actually receive an answer. Of course you're not limited to ping, so you can implement all kinds of network protocols. In general, this implies parsing the received packet, and act accordingly. If using tap, to correctly build reply frames you would probably need to implement ARP in your code. All of this is exactly what User Mode Linux does: it attaches a modified Linux kernel running in userspace to a tap interface that exist on the host, and communicates with the host through that. Of course, being a full Linux kernel, it does implement TCP/IP and ethernet. Newer virtualization platforms like libvirt use tap interfaces extensively to communicate with guests that support them like qemu/kvm; the interfaces have usually names like vnet0, vnet1 etc. and last only as long as the guest they connect to is running, so they're not persistent, but you can see them if you run ip link show and/or brctl show while guests are running.
>In the same way, you can attach with your own code to the interface and practice network programming and/or ethernet and TCP/IP stack implementation. To get started, you can look at (you guessed it) drivers/net/tun.c, functions tun_get_user() and tun_put_user() to see how the tun driver does that on the kernel side (beware that barely scratches the surface of the complete network packet management in the kernel, which is very complex).

### 隧道
下面我们来看看，tuntap接口更牛b的用法。它可以让我们在不需要重新实现TCP/IP协议栈的情况下，实现隧道功能。  
基本原理是什么呢？  
首先，关于什么是隧道，可以看看[这里](http://www.enterprisenetworkingplanet.com/netsp/article.php/3624566/Networking-101-Understanding-Tunneling.htm)，和[这里](https://en.wikipedia.org/wiki/Tunneling_protocol)  



### 本文主要参考：  
[Tun/Tap interface tutorial](http://backreference.org/2010/03/26/tuntap-interface-tutorial/)  
[Inside the Linux Packet Filter](http://www.linuxjournal.com/article/4852)
