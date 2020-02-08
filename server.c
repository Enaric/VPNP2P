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
#include "NodeInfo/node_info.h"
#include "Util/message.h"
#include "CronJob/node_cron_job.h"

#define BUFLEN 2048
#define NUM_THREADS 2      // 最多创建2个子线程, 1个用于接收数据[可能是服务器或者其它peer的](会block), 2个用于定期punching, 并给服务器发消息保持连接.
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

void handle_register(struct Message *msg, int socket_fd) {
    struct Node *recv_node = convert_buf_to_node(msg->buf);
    local_node = file2struct(FILENAME);
    
    // 合并register信息和本地节点信息，并保存
    node_merge(local_node, recv_node);
    struct2file(local_node, FILENAME);
    
    // 生成返回Message
    struct Message reply_msg;
    bzero(&reply_msg, sizeof(reply_msg));
    int len = convert_node_to_buf(local_node, reply_msg.buf);
    reply_msg.size = len;
    strcpy(reply_msg.type, R_REGISTER);
    
    // R_Register
    if (send_msg_over_socket(&reply_msg, socket_fd) < 0) {
        printf("reply register message failed\n");
    }
    printf("Reply Register Message Success!\n");
}

void handle_regular_update(char *recv_ip, int socket_fd) {
    struct Message reply_msg;
    bzero(&reply_msg, sizeof(reply_msg));
    strcpy(reply_msg.buf, REGULAR_UPDATE_RETURN);
    strcpy(reply_msg.type, REGULAR_UPDATE_RETURN);
    reply_msg.size = sizeof(REGULAR_UPDATE_RETURN);
    
    // todo 更新路由信息
    printf("recv ip: %s\n", recv_ip);
    
    if (send_msg_over_socket(&reply_msg, socket_fd) < 0) {
        printf("reply regular update message failed\n");
    }
    printf("Reply Regular Update Message Success!\n");
}

void handle_width_test(int socket_fd) {
    struct Message reply_msg;
    bzero(&reply_msg, sizeof(reply_msg));
    strcpy(reply_msg.buf, WIDTH_TEST_RETURN);
    strcpy(reply_msg.type, WIDTH_TEST_RETURN);
    reply_msg.size = sizeof(WIDTH_TEST_RETURN);
    
    if (send_msg_over_socket(&reply_msg, socket_fd) < 0) {
        printf("reply width test message failed\n");
    }
    printf("Reply Width Test Message Success!\n");
}

// 接收消息的线程，负责根据收到的消息进行处理
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

        time_t t;
        struct tm * lt;
        time (&t);//获取Unix时间戳。
        lt = localtime (&t);//转为时间结构。
        printf("%d/%d/%d %d:%d:%d [%s:%d] %s request from server.\n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), msg.type);
        fflush(stdout);
        
        if (strcmp(msg.type, REGISTER) == 0) {
            handle_register(&msg, new_server_socket_fd);
        } else if (strcmp(msg.type, REGULAR_UPDATE) == 0) {
            handle_regular_update(inet_ntoa(client_addr.sin_addr), new_server_socket_fd);
        } else if (strcmp(msg.type, REGULAR_UPDATE_RETURN) == 0) {
            handle_width_test(new_server_socket_fd);
        }
    }

    return NULL;
}

void handle_router_refresh() {
    struct Node *fromNode;
    struct Node *ptr;
    
    // 先找到对应的本地节点
    for (ptr = local_node; ptr != NULL; ptr = ptr->next_node) {
        if (ptr->id == local_node_id) {
            fromNode = ptr;
            break;
        }
    }
    
    // 向其他所有不是本地节点的节点发送
    for (ptr = local_node; ptr != NULL && ptr->id != local_node_id; ptr = ptr->next_node) {
        router_refresh(fromNode, ptr);
    }
    
    printf("router refreshed\n");
}

void *thread_cron_job(void *arg) {
    while(1) {
        sleep(3600);
        local_ip_refresh(); // 本地ip刷新
        handle_router_refresh(); // 路由、时延、带宽
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
    pthread_create(&(a_thread[1]), NULL, thread_cron_job, NULL);
    pthread_create(&(a_thread[0]), NULL, thread_receive_pack, NULL);  /// 创建线程用于接收客户端发来是数据包
    pthread_join(a_thread[0], NULL);
    pthread_join(a_thread[1], NULL);
    // free_program();
    printf("Complete Successfully.\n");
    return 0;
}
