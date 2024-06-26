#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>


#include "../Util/message.h"
#include "../Util/util.h"
#include "../NodeInfo/node_pair.h"
#include "../NodeInfo/table.h"

#define PORT 3389

// 本地ip刷新
void local_ip_refresh() {
    struct Node *current_node = generate_local_node();
    struct IP *current_ip_list = current_node->ip_list;
    // 比对local_node 中的ip_list 和现在新得到的ip list，并设置各个字段
    struct IP *before_ip_list = local_node->ip_list;
    if (! ip_list_identical(current_ip_list, before_ip_list)) {
        struct IP *ptr;
        for (ptr = current_ip_list; ptr != NULL; ptr = ptr->next_ip) {
            if (contain_ip(before_ip_list, ptr->ip)) {
                ptr->validity = 0;
            } else {
                strcpy(ptr->type, "unknownIP");
                ptr->validity = 1;
            }
        }
    }
    
    // 将local_node中的ip列表替换为刷新得到的ip列表
    local_node->ip_list = current_ip_list;
    printf("local_ip_refreshed\n");
}

// 刷路由
void router_refresh(struct Node *fromNode, struct Node *toNode) {
    struct IP *ip_list = toNode->ip_list;
    struct IP *p = ip_list;
    // 得到toNode ip列表的长度，并创建table
    int length = 0;
    while (p != NULL) {
        length++;
        p = p->next_ip;
    }
    T table = Table_new(length, 0, 0);
    P tmpPair;
    for (p = ip_list; p != NULL; p = p->next_ip) {
        int client_socket_fd = 0;
        client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket_fd < 0) {
            perror("Create socket failed");
            exit(1);
        }
        
        clock_t start, finish;

        // 向服务器发起连接
        if(connect_to_server(client_socket_fd, p->ip, PORT) < 0)
        {
            perror("Can Not Connect To Server IP:");
            exit(0);
        }

        // 构造Message
        struct Message msg;
        struct Message reply_msg;
        strcpy(msg.type, REGULAR_UPDATE);
        strcpy(msg.buf, REGULAR_UPDATE);

        start = clock();
        // 向目标server发送regular update信息
        if (send_msg_over_socket(&msg, client_socket_fd) < 0) {
            printf("send message failed\n");
            exit(1);
        }
        // 接收server传回的信息
        if (recv_msg_over_socket(&reply_msg, client_socket_fd) < 0) {
            printf("receive message failed\n");
            exit(1);
        }
        finish = clock();
        // 2倍的时延，单位为毫秒
        int duration = (int)(finish - start);
        printf("the duration1: %d\n", duration);
        int delay = duration / 2;
        // fromNode --> toNode:p->ip
        P pair = create_node_pair(fromNode, toNode, p->ip);
        tmpPair = pair;
        // todo table中对应的value应该是一个结构体，结构体中放时延和带宽信息
        // Table_put(table, pair, &duration);
        close(client_socket_fd);
        sleep(5);
        
        client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (client_socket_fd < 0) {
            perror("Create socket failed");
            exit(1);
        }
        // 向服务器发起连接
        if(connect_to_server(client_socket_fd, p->ip, PORT) < 0)
        {
            perror("Can Not Connect To Server IP:");
            exit(0);
        }
        struct Message width_reply_msg;
        start = clock();
        // 向目标server发送width test信息
        if (send_file_over_socket(client_socket_fd) < 0) {
            printf("send message failed\n");
            exit(1);
        }
        // 接收server传回的信息
        if (recv_msg_over_socket(&width_reply_msg, client_socket_fd) < 0) {
            printf("receive message failed\n");
            exit(1);
        }
        finish = clock();
        // 2倍的时延，单位为毫秒
        duration = (int)(finish - start);
        printf("the duration2: %d\n", duration);
        // width = 512KB / (finsh - start - delay * 2) 单位为kb/s
        double width = (512.0 / (duration - delay * 2)) * 1000;

        W w = create_width_delay(delay, width);
        
        // 将时延，带宽信息保存到table中
        // key = NodeA-->NodeB:ip, value = delay, width
        Table_put(table, pair, w);
    }
    void *value = Table_get(table, tmpPair);
    printf("the delay is: %d, width: %lf\n", (*(W)value).delay, (*(W)value).width);
}

// int main() {
//    struct Node fromNode;
//    struct Node toNode;
//    struct IP ip;
//    strcpy(ip.ip, "127.0.0.1");
//    ip.next_ip = NULL;
//    toNode.id = 2;
//    toNode.ip_list = &ip;
//
//    fromNode.id = 1;
//    router_refresh(&fromNode, &toNode);
//    struct IP *ip_list = (struct IP *)malloc(sizeof(struct IP));
//    strcpy(ip_list->ip, "127.0.0.2");
//    strcpy(ip_list->type, "clientIP");
//    ip_list->s_count = 0;
//    ip_list->validity = 1;
//
//    struct IP *next_ip = (struct IP *)malloc(sizeof(struct IP));
//    strcpy(next_ip->ip, "127.0.0.3");
//    strcpy(next_ip->type, "serverIP");
//    next_ip->s_count = 0;
//    next_ip->validity = 1;
//
//    ip_list->next_ip = next_ip;
//
//    local_node = (struct Node *)malloc(sizeof(struct Node));
//    local_node->ip_list = ip_list;
//    local_node->node_count = 0;
//    local_node->reliable = 1;
//    local_node->next_node = NULL;
//
//    print_node(local_node);
//    local_ip_refresh();
//    printf("\n\n");
//    print_node(local_node);
// }
