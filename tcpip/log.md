### 2015-12-01
-----------

### 内核通过不同的接口把内部信息输出到用户空间。
* proc 文件系统 (/proc)

* sysctl (系统命令，对应于/proc/sys目录下的文件)

* sysfs 文件系统 (/sys)

* ioctl 系统调用 (socket)

* Netlink 套接字 (socket)
  这是网络应用程序与内核通信时最新的首选机制。

### 内核通知链
内核的订阅-发布者系统，提供给各个子系统间传递事件信息的系统。

### 网络设备初始化
//net/core/dev.c 
// 网络代码的初始化的重要部分，包括流量控制和各个CPU人口队列，该函数实现,在系统引导期间被调用
* net_dev_init


### Virtual Devices
* Possible relationship between virtual and real devices
![]()

#### List of Linux virtual devices
* Bonding  
With this feature, a virtual device bundles a group of physical devices and makes
them behave as one.

* 802.1Q  
This is an IEEE standard that extends the 802.3/Ethernet header with the so-
called VLAN header, allowing for the creation of Virtual LANs.

* Bridging  
A bridge interface is a virtual representation of a bridge.

* Aliasing interfaces  
Originally, the main purpose for this feature was to allow a single real Ethernet
interface to span several virtual interfaces (eth0:0, eth0:1, etc.), each with its own
IP configuration. Now, thanks to improvements to the networking code, there is
no need to define a new virtual interface to configure multiple IP addresses on
the same NIC. However, there may be cases (notably routing) where having dif-
ferent virtual NICs on the same NIC would make life easier, perhaps allowing
simpler configuration.


* True equalizer (TEQL)
This is a queuing discipline that can be used with Traffic Control. Its implemen-
tation requires the creation of a special device. The idea behind TEQL is a bit
similar to Bonding.

* Tunnel interfaces
The implementation of IP-over-IP tunneling (IPIP) and the Generalized Routing
Encapsulation (GRE) protocol is based on the creation of a virtual device.

* Tun/Tap

