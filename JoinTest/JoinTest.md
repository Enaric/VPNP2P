## 服务器:

服务器默认以3389为监听端口(阿里云默认开启的端口).  

./server 3389



## 客户端:

连接服务器

./main target_ip

main函数中默认会调用node_join.c中的join函数  


## 编译命令:

`gcc main.c JoinTest/node_join.c JoinTest/node_info.c Util/message.c Util/util.c -o main`
`gcc node_join_server.c ../Util/message.c -pthread -o server`
