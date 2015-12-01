## QEMU搭建linux kernel 调试环境
* [利用qemu进行内核源码级调试](http://blog.csdn.net/gdt_a20/article/details/7231652)

* [制作镜像](http://minimal.linux-bg.org/)

* [网络配置](http://i.huaixiaoz.com/linux/kvm_qemu.html)
```bash
sudo qemu-system-x86_64 -S -kernel arch/x86/boot/bzImage  -initrd /home/clouder/example/minimal_linux_live/work/rootfs.cpio.gz -append "root=/dev/ram rdinit=/init noapic" -net nic,model=virtio -net tap -m 1024 -s
```  

* [在QEMU中调试ARM程序](http://www.linuxeden.com/html/develop/20100820/104409.html)



## sockect 层
* socket层是用户程序使用处于内核层的tcpip协议栈功能的接口, 用户层程序通过操作socket套接字使用linux的tcpip协议功能。

## 内核中与socket相关的重要数据结构

* 协议族
struct net_protocol_family // 在内核在表示一个套接字协议族
                           // 通过sock_register 注册到 数组net_families 中由内核进行管理 
```cpp
// IPv4 协议族
static const struct net_proto_family inet_family_ops = {
    .family = PF_INET,                                                                                                                                 
    .create = inet_create,  // 创建该协议族的套接字都会通过这个函数来实现
    .owner  = THIS_MODULE,
};

// 注册流程
// net/ipv4/af_inet.c
// init inet_init 函数调用 sock_register 进行注册
```

* 套接字及其操作集 
// include/linux/net.h
struct socket  // 应用层套接字的内核表示

// include/net/sock.h
struct sock    // 套接字在协议族内的表示结构，所有的套接字最后通过该结构来使用网络协议栈的服务

// include/linux/net.h
struct proto_ops // 封装套接字的统一操作集  -- socket 层面

// include/net/sock.h
struct proto   //  封装协议族内各具体协议针对套接字操作集的实现 -- sock 层面
               // 通过proto_register函数注册到proto_list 链表(注册到这里只是给proc文件系统使用)
* 
// include/linux/socket.h
struct msghdr // 应用程序与内核协议族交互网络数据的结构体

* 管理 IPv4 协议族内 上层协议的数据结构 
```cpp
struct inet_protosw
// include/net/protocol.h
/* This is used to register socket interfaces for IP protocols.  */
struct inet_protosw {
    struct list_head list;

        /* These two fields form the lookup key.  */
    unsigned short   type;     /* This is the 2nd argument to socket(2). */
    unsigned short   protocol; /* This is the L4 protocol number.  */

    struct proto     *prot;
    const struct proto_ops *ops;
  
    unsigned char    flags;      /* See INET_PROTOSW_* below.  */
}; 
// 通过 inet_register_protosw 注册到inetsw数组中


// include/net/protocol.h
struct net_protocol // 封装了IPv4 协议族中 网络层向传输层传输数据包的方法
// 通过 inet_add_protocol 函数注册到inet_protos数组中
```


## 如何在内核中创建新的协议族及协议
1. 定义一个net_proto_family
2. 实现其中的create函数
   这个函数需要做什么：
   1. socket->ops (即struct proto_ops)
   2. socket->sk (即struct sock)



-----------
### TCP
// net/ipv4/af_inet.c
struct net_protocol   tcp_protocol

// 传输控制块的继承关系
sock_common
  ^    ^
  |    |
  |    sock <- inet_sock <- inet_connection_sock <- tcp_sock
  |
  request_sock <- inet_request_sock <- tcp_request_sock

// TCP传输控制块，三种
struct tcp_request_sock
struct tcp_sock
struct tcp_timewait_sock

// 面向连接的传输控制块
struct inet_connection_sock // 继承于 struct inet_sock

// 传输层相关的操作集，包括向网络层发送数据，传输层的setsockopt等, tcp的实例为ipv4_specific
inet_connection_sock_af_ops

// TCP选项信息
struct tcp_options_received

// TCP在skb中的私有信息控制块
struct tcp_skb_cb

// TCP proto 操作集和 proto_ops操作集
实例 tcp_prot
实例 inet_stream_ops

// TCP 头部结构
struct tcphdr



## tcp server socket (passive socket)启动过程
------
## socket
## bind 
## listen
## accept

### 网络命名空间
// include/net/net_namespace.h
struct net // 代表一个网络命名空间??


// 允许绑定一个非本地IP
// If set, allows processes to bind() to non-local IP addresses, which can be quite useful,   
// but may break some applications. The default value is 0.

/proc/sys/net/ipv4/ip_nonlocal_bind

## tcp client socket (active socket)启动过程
------
## socket
## connect


