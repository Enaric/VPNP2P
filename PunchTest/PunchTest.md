## 服务器:

服务器默认以9930为监听端口.

./udp_server port respondstring



## 客户端:

连接服务器

./udp_client 1 IP port sendstring < ./udp_client 1 IP [2个参数]

向服务器9930端口每1秒发一次连接请求



连接客户端punch

 ./udp_client 2 IP sleep_time sendstring recvstring (5个参数)  < ./udp_client 2 IP sleep_time [3个参数]

sleep_time一般设置为10000(10毫秒)



## 编译命令:

`gcc -o binwin/udp_client -static -pthread -std=c11 udp_client.c`
`gcc -o binwin/udp_server -static -pthread -std=c11 udp_server.c`

`/home/test/Programs/SynologyDev/toolkit/build_env/ds.apollolake-6.1/bin/x86_64-unknown-linux-gnu-gcc -o binsyno/udp_client udp_client.c -I/home/test/Programs/SynologyDev/toolkit/build_env/ds.apollolake-6.1/usr/include -L/home/test/Programs/SynologyDev/toolkit/build_env/ds.apollolake-6.1/lib -std=c11 -lpthread -static`
`/home/test/Programs/SynologyDev/toolkit/build_env/ds.apollolake-6.1/bin/x86_64-unknown-linux-gnu-gcc -o binsyno/udp_server udp_server.c -I/home/test/Programs/SynologyDev/toolkit/build_env/ds.apollolake-6.1/usr/include -L/home/test/Programs/SynologyDev/toolkit/build_env/ds.apollolake-6.1/lib -lpthread -static`

## Punch History/Log

移动端用到过的IP和端口
183.209.51.212 : 23289
