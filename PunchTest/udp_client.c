#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <float.h>
#include <time.h>

#define BUFLEN 512
#define NPACK 10
#define NUM_THREADS 4      // 最多创建4个子线程, 1个用于接收数据[可能是服务器或者其它peer的](会block), 2个用于定期punching, 并给服务器发消息保持连接.
#define PUNCH_ORI_RATIO 1  // 原始端口的尝试次数比例    通过调整比例来设置随机发送
#define PUNCH_RAND_RATIO 1 // 随机端口的尝试次数比例
#define PORT 65535
// #define SRV_IP "47.100.48.35"//阿里云服务器1
// #define SRV_IP "47.100.170.4"//阿里云服务器2
// #define SRV_IP "218.94.159.98"//学院电信网
// **校园网ip地址不稳定
// #define SRV_IP "211.162.81.112"//机房校园网
 // #define SRV_IP "202.119.40.121"// 宿舍校园网313
// #define SRV_IP "202.119.46.16"// 宿舍校园网312

struct client
{
    int host;
    uint16_t port;
};

int boolean_cond;                    // 用于条件判断
pthread_t a_thread[NUM_THREADS];     // 缓存线程
uint8_t threadflag;                      // 线程直接用于交互的全局信号变量, 默认:0, 程序退出:1;
int s;                               // 用于监听 socket 的文件描述符
struct sockaddr_in si_other;         // 服务器和网络收到的IP地址
struct sockaddr_in test_other;       // 服务器和网络收到的IP地址
unsigned int slen;                   // 地址占用的字节数
char buf[BUFLEN];                    // 网络接收的数据缓存
char SRV_IP[100];
char send_string[100];
char recv_string[100];
int type;
int usleep_time;
int server_port;
int heartbeat = 0;
int if_print = 1;

int free_program() {
    if(-1 != s){
        close(s);
        s = -1;
    }
    return 0;
}

// Just a function to kill the program when something goes wrong.
void diep(char *message) {
    printf("!Error: %s, exit\n",message);fflush(stdout);
    perror(message);
    free_program();
    exit(1);
}

int init_program() {
    boolean_cond = 0;
    threadflag = 0;
    s = -1;
    slen = sizeof(struct sockaddr_in);
    if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)  //s是一个接收数据包的网络监听器
        diep("socket");
    //设定服务器IP和端口  The server's endpoint data
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    if (inet_aton(SRV_IP, &si_other.sin_addr) == 0)
        diep("aton");

    memset((char *) &test_other, 0, sizeof(test_other));
    test_other.sin_family = AF_INET;
    test_other.sin_port = htons(PORT);
    if (inet_aton(SRV_IP, &test_other.sin_addr) == 0)
        diep("aton");
    return 0;
}

void *thread_heartbeat(void *arg) {
    char tempbuf[128]; 
    int reclen;
    uint16_t times = 0;
    while (1) //通过轮询的方式来监听
    {
        if(heartbeat == 1){
            times++;
            uint32_t *cnt = (uint32_t*)inet_ntoa(si_other.sin_addr);
            if (sendto(s, send_string, 6, 0, (struct sockaddr*)(&si_other), slen) == -1){
                printf("send fail for some unknown reason");fflush(stdout);
            }
            printf("received,send to %s:%d , heartbeat times : %d\n", SRV_IP,ntohs(si_other.sin_port),times);fflush(stdout);
            
            sleep(5);
        } 
    }
}

/// 接收数据的线程
void *thread_receive_pack(void *arg) {
    char tempbuf[128]; 
    int reclen;
    uint16_t times = 0;
    while (1) //通过轮询的方式来监听
    {
        reclen = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)(&si_other), (socklen_t*)(&slen));
        if (reclen == -1) { /// 收到一个数据包, 可能会来自服务器, 也可能会来自peer
            printf("Error: recvfrom return error\n");fflush(stdout);
            continue;
        }
        for(int i=0;i<reclen;++i){ tempbuf[i]=buf[i]; } tempbuf[reclen]='\0';
        if(strcmp(tempbuf, recv_string) != 0){
            printf("Error: other packet[%s:%d]:%s \n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);fflush(stdout);
            continue;
        }
        times++;
        printf("Maybe Successful Received from Peer %s:%d! packet:%s \n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
        fflush(stdout);
        threadflag = 1;
        heartbeat = 1;

    }
}

void *thread_punching(void *arg) {
    uint16_t cnt = 0;
    uint32_t times = 0;
    // printf("%d\n", type);
    if(type == 1){
        while (1) {
            sleep(5);
            times++;
            if (sendto(s, send_string, 6, 0, (struct sockaddr*)(&test_other), slen) == -1){
                printf("send fail for some unknown reason");fflush(stdout);
            }
            printf("send to %s: %d ,times : %d\n", SRV_IP,server_port,times);fflush(stdout);
            test_other.sin_port = htons(9930);
            if(1 == threadflag){
                pthread_exit(NULL);
            }
        }
    }else{
        while (1) {
            usleep(usleep_time);
            times++;
            if (sendto(s, send_string, 6, 0, (struct sockaddr*)(&test_other), slen) == -1){
                printf("send fail for some unknown reason");fflush(stdout);
            }
            if(if_print==1){
                printf("send to %s: %d ,times : %d\n", SRV_IP,PORT-cnt,times);fflush(stdout);
                if_print = 0;
            }
            test_other.sin_port = htons(PORT - cnt);
            cnt = (cnt + 1) % 65535;
            if(1 == threadflag){
                pthread_exit(NULL);
            }
        }
    }
    
    
}

void *count_time(void *arg) {
    uint16_t times = 0;
    while (1) //通过轮询的方式来监听
    {
        sleep(1);
        times++;
        if(times%15 == 0){
            if_print = 1;
        }else{
            if_print = 0;
        }
    }
}

int main(int argc,char *argv[])
{
    type = atoi(argv[1]);//1连接客户端 2连接服务器
    if(type==1){
        if (argc != 5){
            printf("请检查参数个数");
            return 0;
        }
        strcpy(SRV_IP,argv[2]);//IP地址
        server_port = atoi(argv[3]);//目标端口
        strcpy(send_string,argv[4]);//send_string
    }else{
        if (argc != 6){
            printf("请检查参数个数");
            return 0;
        }
        strcpy(SRV_IP,argv[2]);//IP地址
        usleep_time = atoi(argv[3]);//sleep_time
        strcpy(send_string,argv[4]);//send_string
        strcpy(recv_string,argv[5]);//recv_string
    }
    init_program();
    int res;
    res = pthread_create(&(a_thread[0]), NULL, thread_receive_pack, NULL);  /// 创建线程用于接收客户端发来是数据包
    if (res != 0) {
        diep("receive_pack Thread creation failed");
    }
    res = pthread_create(&(a_thread[1]), NULL, thread_punching, NULL);      /// 发送线程
    if (res != 0) {
        diep("broadcast_pack Thread creation failed");
    }
    res = pthread_create(&(a_thread[2]), NULL, thread_heartbeat, NULL);      /// 心跳包
    if (res != 0) {
        diep("thread_heartbeat Thread creation failed");
    }
    res = pthread_create(&(a_thread[3]), NULL, count_time, NULL);      /// 计时器
    if (res != 0) {
        diep("count_time Thread creation failed");
    }
    for(int i = 3 - 1; i >= 0; i--) { //从后往前join
        res = pthread_join(a_thread[i], NULL);
        if (res == 0) {
           printf("Join a thread\n");
        }
        else {
            diep("pthread_join failed");
        }
    }
    free_program();
    printf("Complete Successfully.\n");
    return 0;
}
