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
#define NUM_THREADS 2      // 最多创建2个子线程, 1个用于接收数据[可能是服务器或者其它peer的](会block), 2个用于定期punching, 并给服务器发消息保持连接.
#define PUNCH_ORI_RATIO 1  // 原始端口的尝试次数比例    通过调整比例来设置随机发送
#define PUNCH_RAND_RATIO 1 // 随机端口的尝试次数比例
#define PORT 1
#define SRV_IP "127.0.0.1"
#define numOfPort 10  // 服务器所开端口个数  ！！自定义

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
int Listen_PORT;
char respond_string[100];

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

    struct sockaddr_in si_me;
    memset((char*) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(Listen_PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);
    bind(s, (struct sockaddr*)(&si_me), sizeof(si_me));

    //设定服务器IP和端口  The server's endpoint data
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(PORT);
    if (inet_aton(SRV_IP, &si_other.sin_addr) == 0)
        diep("aton");

    return 0;
}

/// 接收数据的线程
void *thread_receive_pack(void *arg) {
    char tempbuf[128]; 
    int reclen;
    while (1) //通过轮询的方式来监听
    {
        reclen = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr*)(&si_other), (socklen_t*)(&slen));
        if (reclen == -1) { /// 收到一个数据包, 可能会来自服务器, 也可能会来自peer
            printf("Error: recvfrom return error\n");fflush(stdout);
            continue;
        }
        if(1 != threadflag){ /// 包是从peer收到的
            time_t t;
            struct tm * lt;
            time (&t);//获取Unix时间戳。
            lt = localtime (&t);//转为时间结构。
            printf("%d/%d/%d %d:%d:%d [%s:%d] packet:%s \n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
            fflush(stdout);
            if (sendto(s,respond_string, 6, 0, (struct sockaddr*)(&si_other), slen) == -1){
                printf("send fail for some unknown reason");fflush(stdout);
            }
            // threadflag = 1;
        }
        if(1 == threadflag){
            pthread_exit(NULL);
        }
    }
}

int main(int argc,char *argv[])
{
    if (argc != 3){
        printf("请检查参数个数");
        return 0;
    }
    Listen_PORT = atoi(argv[1]);//目标端口
    strcpy(respond_string,argv[2]);//send_string
    init_program();
    int res;
    res = pthread_create(&(a_thread[0]), NULL, thread_receive_pack, NULL);  /// 创建线程用于接收客户端发来是数据包
    res = pthread_join(a_thread[0], NULL);
    if (res != 0) {
        diep("receive_pack Thread creation failed");
    }
    // free_program();
    printf("Complete Successfully.\n");
    return 0;
}
