## sk_buff
sk_buff is the network buffer that represents the network packet on Linux TCP/IP stack.
sk_buff has three components: sk_buff, and linear - data buffer, and paged data(struct skb_shared_info )
Whenever we allocate a new sk_buff, we provide the size of the linear data area.

sk_buff consists of three segments:
* sk_buff structure, which is also referred to as a sk_buffer **header**
* **Linear** data block containing data
* **Nonlinear data portion represented by struct skb_shared_info


