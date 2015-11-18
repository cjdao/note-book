## 可靠传输原理
* 可靠数据传输协议是建立在ARQ(Automatic Repeat reQuest--自动重复请求)的协议之上的
　在ARQ协议中，基本上需要３种额外的协议功能来处理比特错误的出现:
  * 错误检测
  * 接收方反馈
  * 重传

* [停止等待协议(stop-and-wait)](http://www.erg.abdn.ac.uk/users/gorry/eg3567/arq-pages/saw.html)

* [交替比特协议(alternating-bit protocol)]()


* 为了解决停止等待协议效率低下的问题，引进了**流水线技术**，该技术需要对停止等待协议进行以下改进：
　* 序列号编号上限必须增加
　* 协议的发送方和接收方必须能缓存多个分组




[TCP/IP重传超时--RTO](http://www.orczhou.com/index.php/2011/10/tcpip-protocol-start-rto/)


[Congestion control](http://ecomputernotes.com/computernetworkingnotes/communication-networks/what-is-congestion-control-describe-the-congestion-control-algorithm-commonly-used)
