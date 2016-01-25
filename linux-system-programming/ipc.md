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
    * 字节流: 管道、FIFO、数据报socket(???)是无分隔符的字节流
    * 消息: System V消息队列、POSIX消息队以及数据报socket(unix socket?)
    * 伪终端: 在特殊情况下使用的通信工具
  * 共享内存
    * System V 共享内存
    * Posix 共享内存
    * 内存映射(匿名映射及文件映射)

* 使用共享内存的注意点
  * 放入共享内存的数据，对所有共享这块内存的进程可见
  * 多进程使用共享内存，一般都需要使用信号量来同步

* 数据传输工具与共享内存的差别
  * 尽管一个数据传输工具能有多个读者，但读取操作是具有破坏性的。读取操作会消耗数据，其他进程无法在获取被消耗的数据。
  * 数据传输工具的读取者和写者进程间的同步是原子的，如果一个读取者试图从一个当前不包含数据
  * 数据传输工具要求在用户内存和内核内存间进行两次数据传输

### 同步工具 
* 信号量: System V信号量和POSIX信号量
* 文件锁: 文件锁是设计用来协调操作同一个文件的多个进程的
* 互斥体和条件变量(作用于线程)

### IPC 工具比较
* IPC对象标识符和其句柄
![](https://github.com/cjdao/note-book/blob/master/images/unixipchandle.png)

* 功能
* 网络通信
* 

### 两种IPC
* System V IPC mechanism
* Posix IPC
  
### Posix IPC 三种方式
* message queues
* semaphores
* shared memory

### Posix IPC接口汇总
![](https://github.com/cjdao/note-book/blob/master/images/)
