# 43.进程间通信简介
----------------------
### Unix系统上各种通信和同步工具
* 根据功能可分成三个类别:   
 * 通信：关注进程间的**数据交换**
 * 同步：关注进程和线程间的操作**同步**
 * 信号：可以作为一种同步技术，或者更罕见的作为一种通信技术
![](https://github.com/cjdao/note-book/blob/master/images/UnixIpcCategory.png)

### 通信工具
* 主要用于进程间交换数据
* 也可以用于进程中线程间交换数据，但一般不这么做，因为线程间可以通过共享全局变量来交换数据
* 通信工具可以分成两类：  
  * 数据传输工具
    
    * 字节流: 管道、FIFO、数据包socket是无分隔符的字节流
    * 消息: System V消息队列、POSIX消息队S
    * 伪终端
  * 共享内存
    * System V 共享内存
    * Posix 共享内存
    * 内存映射

### 同步工具 
* 信号量
* 文件锁
* 互斥体和条件变量(作用于线程)


### IPC 工具比较
* IPC对象标识符和其句柄
![](https://github.com/cjdao/note-book/blob/master/images/unixipchandle.png)

* 功能

* 网络通信

* :
### 两种IPC
* System V IPC mechanism
* Posix IPC
  
### Posix IPC 三种方式
* message queues
* semaphores
* shared memory

### Posix IPC接口汇总
![](https://github.com/cjdao/note-book/blob/master/images/)
