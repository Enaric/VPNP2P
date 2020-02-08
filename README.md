# VPNP2P

## 编译命令:

client:  

`gcc main.c InitTest/node_start.c JoinTest/node_join.c NodeInfo/node_info.c Util/message.c Util/util.c -o main`
  
server:  

`gcc server.c Util/message.c Util/util.c NodeInfo/node_info.c CronJob/node_cron_job.c NodeInfo/table.c NodeInfo/node_pair.c -o server`
