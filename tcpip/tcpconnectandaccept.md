##
我们知道，tcp socket的标准使用流程

client:
socket -> connect -> write/read -> close

server:
socket -> listen -> accept -> read/write -> close

在上面的tcp socket标准调用流程中，隐含着一条tcp连接而言，必经历的三个阶段:  
1. 建立连接 -- 三次握手
2. 数据传输
3. 销毁连接 -- 四次/三次握手
