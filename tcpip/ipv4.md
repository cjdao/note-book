## socket 套接字与内核中的协议族

### 内核中的协议族
* struct net_proto_family 代表内核中的一个协议族, 通过sock_register函数注册到内核的net_families数组中
* 内核常见的协议族有：IPv4 IPv6 Unix Netlink 等(参考include/linux/socket.h文件中PF_XXX的定义列表)
* ipv4协议族的struct net_proto_family的实例是inet_family_ops(net/ipv4/af_inet.c)
* struct net_proto_family的定义非常简单  
```cpp
// include/linux/net.h
struct net_proto_family {                                                                                                                 
    int     family;
    int     (*create)(struct net *net, struct socket *sock,
                  int protocol, int kern);
    struct module   *owner;
};
```
 * family 指定协议族，参见include/linux/socket.h文件中PF_XXX的定义列表
 * create socket 系统调用创建socket文件时，最终通过该接口实现, 参考net/socket.c中sock_create函数的实现
 * owner 基本上赋值为THIS_MODULE  
你要在内核实现一个协议族，第一步就是实现一个struct net_proto_family实例，并通过sock_register注册给内核

通过sock_register注册协议族后，用户程序就可以通过socket系统调用创建socket文件了，  
每当用户创建一个socket文件, 内核就会对应创建两个重要的实例（struct socket 和struct sock）  
它们长的很像啊，说明其关系也是相当密切的：  
// include/linux/net.h
struct socket  // 应用层套接字的内核表示
// include/net/sock.h
struct sock    // 套接字在协议族内的表示结构，所有的套接字最后通过该结构来使用网络协议栈的服务

看看它们的定义片段
```cpp
struct socket {
//...
    struct sock     *sk;
//..
};

struct sock {
//...
    struct socket       *sk_socket;
//..
};
```
它们是手拉着手的好兄弟啊！  
* 它们是如何牵手的？    
在net_proto_family实例的create函数发起的调用链中，都会通过调用sock_init_data函数 (net/core/sock.c)让他们牵手(可看看ipv4协议族的实现)

* 内核为啥要搞这么一对宝贝出来呢？    
用户层通过socket系统调用创建了套接字后，还需要进行其他的操作来实现网络通信，而这其他的操作就需要socket和sock来支持了。  
再看看它们的定义片段
```cpp
struct socket {
//...
    struct sock     *sk;
    const struct proto_ops  *ops;  // 新增  
//..
};

struct sock {
//...
    struct socket       *sk_socket;
    struct proto        *sk_prot; // 新增 
//..
};
```

struct proto_ops 定义在include/linux/net.h文件中，里面基本上都是函数指针，而且基本与系统调用接口一一对应，所以这套接口可以看层是用户层到socket层的接口  
struct proto 定义在include/net/sock.h中，内部也有很多函数指针，但它们看起来与struct proto_ops有些相似，只是更复杂一些。这套接口可以看做是socket层与具体的传输层协议的接口。
一般比较复杂的协议族，会对各自的子协议定义各自的proto_ops和proto实例，然后让proto_ops中的函数调用proto中的函数，以实现实践功能(如af_inet.c,PF_INET的实现)。
不过如果比较简单，则可以只实现proto_ops实例内的各函数即可了, proto内就不需要实现任何函数指针了(如af_unix.c内PF_UNIX协议的实现).这个时候为什么还需要sock实例呢？

* 怎么创建socket实例和sock实例

内核提供了两个通用函数用于完成此任务：    
sock_alloc -- 创建socket实例  
sk_alloc -- 创建sock实例  
在用户层程序执行socket系统调用后，这两个实例就会被创建出来。  
socket实例在\__sock_create中被创建，随后，\__sock_create根据协议族调用注册到内核的net_families中的net_proto_family实例中create函数，   
在create函数中sock实例会被创建出来，并且最终通过调用sock_init_data函数，让两者成功牵手。  


### IPv4协议族
* 先找到其net_proto_family实例
```cpp
// net/ipv4/af_inet.c
static const struct net_proto_family inet_family_ops = { 
    .family = PF_INET,
    .create = inet_create,
    .owner  = THIS_MODULE,
};
```
* 再看其注册net_proto_family实例的地方  
```cpp
// net/ipv4/af_inet.c
// ipv4 协议族 的初始化入口函数:  
static int __init inet_init(void) 
{
// ...
    /*
     *  Tell SOCKET that we are alive...
    */

    (void)sock_register(&inet_family_ops);
// ...
}
fs_initcall(inet_init);
```

* 下面透过inet_create函数看看如何创建socket套接字   
我们可以在inet_create函数看到我们已经熟悉的几个点
```cpp
static int inet_create(struct net *net, struct socket *sock, int protocol,
               int kern)
{
    // 1. inet_create是由__sock_create调用的，所以socket实例已经创建了，并通过sock指针参数传递给inet_create
    // ...
    // 2. 给socket实例绑定proto_opss实例
    sock->ops = answer->ops;  // 当然我们目前还不知道answer->ops是怎么来的
    // ...
    // 3. 创建sock实例，并绑定proto实例answer_prot
    sk = sk_alloc(net, PF_INET, GFP_KERNEL, answer_prot, kern); // 我们也还不指定answer_prot是怎么来的
    // ...
    // 4. 关联socket实例和sock实例
    sock_init_data(sock, sk);
    // ...
    // 5. 如果可以，调用sock实例中的init函数，让实际协议对socket做初始化工作
    if (sk->sk_prot->init) {
        err = sk->sk_prot->init(sk);
        if (err)
            sk_common_release(sk);
    }  
    // ...
}
```

由前面的介绍，我们指定对于一个socket套接字而言，在内核中真正负责实际工作的是struct proto_ops和struct proto所指向的实例。
那么这两个实例，在inet_create中是怎么创建的呢?

在ipv4中，所有的实际协议实现都在数组inetsw_array(net/ipv4/af_inet.c)中进行描述:
```cpp
/* Upon startup we insert all the elements in inetsw_array[] into
 * the linked list inetsw.
 */
static struct inet_protosw inetsw_array[] =
{
    {   // TCP 协议
        .type =       SOCK_STREAM,
        .protocol =   IPPROTO_TCP,
        .prot =       &tcp_prot,
        .ops =        &inet_stream_ops,
        .no_check =   0,
        .flags =      INET_PROTOSW_PERMANENT |
                  INET_PROTOSW_ICSK,
    },

    {   // UDP 协议
        .type =       SOCK_DGRAM,
        .protocol =   IPPROTO_UDP,
        .prot =       &udp_prot,
        .ops =        &inet_dgram_ops,
        .no_check =   UDP_CSUM_DEFAULT,
        .flags =      INET_PROTOSW_PERMANENT,
       },

       { // ICMP
        .type =       SOCK_DGRAM,
        .protocol =   IPPROTO_ICMP,
        .prot =       &ping_prot,
        .ops =        &inet_dgram_ops,
        .no_check =   UDP_CSUM_DEFAULT,
        .flags =      INET_PROTOSW_REUSE,
       },

       { // IPv4
           .type =       SOCK_RAW,
           .protocol =   IPPROTO_IP,    /* wild card */
           .prot =       &raw_prot,
           .ops =        &inet_sockraw_ops,
           .no_check =   UDP_CSUM_DEFAULT,
           .flags =      INET_PROTOSW_REUSE,
       }
};
```
这个inetsw_array数组的内容会在IPv4协议初始化期间，被插入inetsw链表中。  
先看看inetsw
```cpp
/* The inetsw table contains everything that inet_create needs to
 * build a new socket.
 */
static struct list_head inetsw[SOCK_MAX];
```
这是一个链表数组，每一种类型的socket对于一条链表，因此这个链表数组包含了所有IPv4协议相关的东东.
**inet_register_protosw()**(net/ipv4/af_inet.c)函数负责将inetsw_array数组中对应的项，注册到inetsw中。
> 为什么要把静态数组inetsw_array里的内容，倒腾到动态链表inetsw中?
> linux内核支持动态加载额外的协议实现!!!

回过头来，看看inet_create的实现，它会遍历inetsw中对应的类型的链表，找到适合的struct proto_ops和struct proto实例,
具体到tcp socket的话，struct proto_ops的实例为inet_stream_ops, struct proto实例为tcp_prot.


