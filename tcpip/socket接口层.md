## socket接口层
socket层复制联通应用层程序，与内核层实现的TCPIP协议实现。

socket被当成文件来对待，不过它的创建不是通过open系统调用而是通过socket系统调用。  
与其他文件操作类似，用户创建完socket文件后会得到一个文件句柄，后续针对该socket  
文件的操作都通过该句柄标识。    

* socket 系统调用流程图  
![](https://github.com/cjdao/note-book/blob/master/images/socket_systemcall_flow.png)

从内核的角度来看socket的话，就没有用户层那么简单了
