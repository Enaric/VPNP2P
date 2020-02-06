# VPNP2P

## 编译命令:

client:  

`gcc main.c InitTest/node_start.c JoinTest/node_join.c NodeInfo/node_info.c Util/message.c Util/util.c -o main`
  
server:  

`gcc JoinTest/node_join_server.c Util/message.c NodeInfo/node_info.c Util/util.c -pthread -o server`