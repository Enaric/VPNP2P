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
#include "../NodeInfo/node_info.h"
#include "../Util/message.h"

#define BUFLEN 2048
#define NUM_THREADS 2      // 最多创建2个子线程, 1个用于接收数据[可能是服务器或者其它peer的](会block), 2个用于定期punching, 并给服务器发消息保持连接.
#define PORT 1099
#define SRV_IP "127.0.0.1"
#define FILENAME "node_info.txt"

int boolean_cond;                    // 用于条件判断
pthread_t a_thread[NUM_THREADS];     // 缓存线程
uint8_t threadflag;                  // 线程直接用于交互的全局信号变量, 默认:0, 程序退出:1;
int server_socket_fd;
struct sockaddr_in server_addr;
int Listen_PORT;
struct Node *local_node;

int free_program() {
    if(-1 != server_socket_fd){
        close(server_socket_fd);
        server_socket_fd = -1;
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
    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
    server_addr.sin_port = htons(Listen_PORT);

    server_socket_fd = socket(PF_INET, SOCK_STREAM, 0);
    if(server_socket_fd < 0) {
        perror("Create Socket Failed:");
        exit(1);
    }
    int opt = 1;
    setsockopt(server_socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    if(-1 == (bind(server_socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)))) {
        perror("Server Bind Failed:");
        exit(1);
    }

     // socket监听
    if(-1 == (listen(server_socket_fd, 20))) {
        perror("Server Listen Failed:");
        exit(1);
    }
    return 0;
}

/// 接收数据的线程
void *thread_receive_pack(void *arg) {
    while (1) //通过轮询的方式来监听
    {
        // 定义客户端的socket地址结构
        struct sockaddr_in client_addr;
        socklen_t client_addr_length = sizeof(client_addr);
 
        // 接受连接请求，返回一个新的socket(描述符)，这个新socket用于同连接的客户端通信
        // accept函数会把连接到的客户端信息写到client_addr中
        int new_server_socket_fd = accept(server_socket_fd, (struct sockaddr*)&client_addr, &client_addr_length);
        if(new_server_socket_fd < 0)
        {
            perror("Server Accept Failed:");
            break;
        }
        
        struct Message msg;
        if (recv_msg_over_socket(&msg, new_server_socket_fd) < 0) {
            printf("receive message failed\n");
            continue;
        }

        if(1 != threadflag){ /// 包是从peer收到的
            time_t t;
            struct tm * lt;
            time (&t);//获取Unix时间戳。
            lt = localtime (&t);//转为时间结构。
            printf("%d/%d/%d %d:%d:%d [%s:%d] register request from server.\n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            fflush(stdout);

            // 生成返回Message
            struct Message reply_msg;
            bzero(&reply_msg, sizeof(reply_msg));
            strcpy(reply_msg.type, msg.type);
            strcpy(reply_msg.buf, msg.buf);

            if (send_msg_over_socket(&reply_msg, new_server_socket_fd) < 0) {
                printf("send message failed\n");
            }
            printf("File:%s Transfer Successful!\n", FILENAME);

            // threadflag = 1;
        }
        if(1 == threadflag){
            pthread_exit(NULL);
        }
    }

    return NULL;
}

int main(int argc,char *argv[])
{
    if (argc != 2){
        printf("请检查参数个数");
        return 0;
    }
    Listen_PORT = atoi(argv[1]);//目标端口
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