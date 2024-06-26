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
#include "../Util/util.h"
#include "../Util/message.h"
#include "../NodeInfo/node_info.h"

#define NPACK 10
#define NUM_THREADS 4      // 最多创建4个子线程, 1个用于接收数据[可能是服务器或者其它peer的](会block), 2个用于定期punching, 并给服务器发消息保持连接.
#define SRV_IP "127.0.0.1"
#define PORT 3389
#define FILENAME "node_info.txt"
#define BUFLEN 2048

pthread_t a_thread[NUM_THREADS];     // 缓存线程
uint8_t threadflag;                  // 线程直接用于交互的全局信号变量, 默认:0, 程序退出:1;

// 注册函数 server传回的集群信息会保存到node中
struct Node* register_node(char *target_ip) {
    // 向服务器发起连接
    int client_socket_fd;
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd < 0) {
        perror("Create socket failed");
        exit(1);
    }
    if(connect_to_server(client_socket_fd, target_ip, PORT) < 0)
    {
        perror("Can Not Connect To Server IP:");
        exit(0);
    }

    // 读入本地节点信息
    struct Message message;
    strcpy(message.type, "Register");
    read_file_to_buf(&message, FILENAME);

    printf("message content: %s\n", message.buf);

    // 向目标server发送register信息
    if (send_msg_over_socket(&message, client_socket_fd) < 0) {
        // printf("send message failed\n");
        perror("Send Message Failed:");
        exit(1);
    }
    
    // 接收server传回的信息
    struct Message reply_msg;
    if (recv_msg_over_socket(&reply_msg, client_socket_fd) < 0) {
        printf("receive message failed\n");
        exit(1);
    }

    // 保存服务端返回消息中的节点信息
    write_buf_to_file(&reply_msg, FILENAME, reply_msg.size - 1);

    time_t t;
    struct tm * lt;
    time (&t);//获取Unix时间戳。
    lt = localtime (&t);//转为时间结构。
    printf("%d/%d/%d %d:%d:%d [%s:%d] receive from server\n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec, inet_ntoa(server_addr.sin_addr), ntohs(server_addr.sin_port));
    fflush(stdout);

    close(client_socket_fd);

    struct Node *ret_node = (struct Node*)malloc(sizeof(struct Node));
    ret_node = convert_buf_to_node(reply_msg.buf);
    
    return ret_node;
}

int send_recheck_msg(char *target_ip) {
    struct Message msg;
    bzero(&msg, sizeof(msg));
    strcpy(msg.type, "Recheck");
    msg.size = 0;

    int client_socket_fd;
    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd < 0) {
        perror("Create socket failed");
        exit(1);
    }
    
    if(connect_to_server(client_socket_fd, target_ip, PORT) < 0)
    {
        perror("Can Not Connect To Server IP:");
        exit(0);
    }

    if (send_msg_over_socket(&msg, client_socket_fd) < 0) {
        printf("send recheck msg failed\n");
        return -1;
    }

    close(client_socket_fd);

    return 1;
}

// 遍历local_node 的ip列表，确认每个local ip都存在
// 如果缓存中ip和本地ip对应，返回true，否则返回false
int judge_local_ip() {
    struct IP *ip;
    char **node_ip_list = malloc(IP_LIST_SIZE * sizeof(char*));
    int pos = 0;
    for (ip = local_node->ip_list;ip != NULL;ip = ip->next_ip) {
        node_ip_list[pos] = ip->ip;
        pos++;
    }

    if (ip_match(node_ip_list, pos)) {
        // 沿用之前的infofile
        // todo 更新打洞信息
        return 1;
    } 
    
    if (node_ip_list != NULL) { free(node_ip_list); node_ip_list = NULL; } 
    return 0;
}

// 向集群中加入新的node
// server_ip 目标server的ip
// use_cache 是否使用缓存
int join(char *server_ip, int use_cache) {
    if (!fopen(FILENAME, "r")) {
        // 不存在infofile
        // 生成本地infofile
        printf("infofile not exist\n");
        generate_local_node();
        print_node(local_node);
        struct2file(local_node, FILENAME);
    } else {
        printf("infofile exist\n");
    }
    
    
    if (use_cache) {
        // 校对本地集群缓存信息
        if(!judge_local_ip()) {
            // 更新 node count
            local_node->node_count += 1;
            free(local_node->ip_list);
            local_node->ip_list = NULL;
            free(local_node->next_node);
            local_node->next_node = NULL;

            struct2file(local_node, FILENAME);
        }
    }

    // 向server发送注册信息
    // todo 保存一份server发回的node信息
    struct Node *server_node = register_node(server_ip);
    
    // todo 如果发生冲突
    printf("LOCAL NODE: \n");
    print_node(local_node);
    // 合并本地集群信息和server传回的集群信息
    node_merge(local_node, server_node);
    
    struct Node *tmp_node = (struct Node*)malloc(sizeof(struct Node));
    struct Node *p;
    for (p = server_ip;p != NULL;p = p->next_node) {
        struct IP *ip;
        for (ip = p->ip_list;ip != NULL;ip = ip->next_ip) {
            if (strcmp(ip->ip, server_ip) == 0) {
                printf("is server ip, should skip\n");
                continue;
            }
            if (is_intranet(ip->ip)) {
                tmp_node = register_node(ip->ip);
                if(node_merge(local_node, tmp_node) == -1) {
                    // todo recheck
                    send_recheck_msg(ip->ip);
                }
            }
        }
    }

    struct2file(local_node, FILENAME);

    if (server_node == NULL) { free(server_node); server_node = NULL; }
    return 0;
}

