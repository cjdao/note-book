##
tcp socket的标准使用流程

client:
socket -> connect -> write/read -> close

server:
socket -> bind -> listen -> accept -> read/write -> close

在上面的tcp socket标准调用流程中，隐含着对于一条tcp连接而言，必经历的三个阶段:  
1. 建立连接 -- 三次握手
2. 数据传输
3. 销毁连接 -- 四次/三次握手

### tcp传输控制块 
* TCP传输控制块在TCP整个过程中起着核心的作用，包括建立连接、数据的传输、拥塞控制以及连接的终止。
在TCP的整个连接过程中，按顺序分别使用以下三种类型的TCP传输控制块：
1. tcp_request_sock , 建立连接的过程中使用，存在的时间比较短
2. tcp_sock, 在连接建立之后终止之前使用，TCP状态为ESTABLISHED
3. tcp_timewait_sock，在终止连接的过程中使用，其存在的时间也比较短

* ipv4 中struct sock的继承关系:
sock_common
 ^   ^   ^
 |   |   |
 |   |   sock <- inet_sock <- inet_connection_sock <- tcp_sock
 |   |                 ^
 |   |                 |_____________________________ udp_sock 
 |   |
 |   request_sock <- inet_request_sock <------ tcp_request_sock
 | 
 inet_timewait_sock <------------------------ tcp_timewait_sock

* inet_connection_sock 所有面向连接传输控制块的表示，在inet_sock的基础上，增加有关连接、确认和重传等成员。


* 从服务端的角度看，一条tcp连接的建立，需经历以下几个系统调用:socket -> bind -> listen -> accept
其中socket是建立套接字
bind是绑定地址及端口号
listen进入监听状态，等待客户端的syn报文
而accept则是让用户进程进入数据读写阶段

* tcp服务端，与tcp连接建立过程相关的数据结构
request_sock_queue
listen_sock
tcp_request_sock
request_sock_ops

* bind 系统调用的实现


