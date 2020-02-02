#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include "../Util/util.h"
#include "../Util/message.h"
#include "node_info.h"

int client_socket_fd;
struct sockaddr_in client_addr;
struct sockaddr_in server_addr;
// int Listen_PORT;
struct Node *local_node;             // 本地节点

int free_program() {
    if(-1 != client_socket_fd){
        close(client_socket_fd);
        client_socket_fd = -1;
    }
    return 0;
}

int init_socket() {
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0); 

    client_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket_fd < 0) {
        perror("Create socket failed");
        exit(1);
    }
}

int generate_info() {
    // 标定内网ip
    // 生成一个全局的节点信息数据结构
    // 可以提供给后面的流程使用
    struct Node tmp_node;
    bzero(&tmp_node, sizeof(tmp_node));
    tmp_node.id = generate_id(); // todo 全局唯一id生成器
    tmp_node.node_count = 1;
    tmp_node.reliable = RELIABLE;
    struct IP *ip = (struct IP *)malloc(sizeof(struct IP));
    tmp_node.ip_list = ip;
    char **local_ip_list = get_local_ip();
    for(int i = 0;i < IP_LIST_SIZE;i++) {
        if (local_ip_list[i] == NULL) {
            break;
        }
        strcpy(ip->ip, local_ip_list[i]);
        if (is_intranet(local_ip_list[i])) {
            strcpy(ip->type, "clientIP");
        } else {
            strcpy(ip->type, "unknownIP");
        }
        ip->s_count = 0;

        if (local_ip_list[i+1] != NULL) {
            struct IP ip_new;
            bzero(&ip_new, sizeof(ip_new));
            ip->next_ip = &ip_new;
            ip = &ip_new;
        }
    }
    // 将生成的节点信息写到文件
    struct2file(&tmp_node, "InitTest/node_info.txt");

    if (ip != NULL) { free(ip); ip = NULL; }
    if (local_ip_list != NULL) { free(local_ip_list); local_ip_list = NULL; }
    return 0;
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

// 初始搭建，第一个Node必须指定一个serverIP
//./net start serverIP1 [, serverIP2, infofile=xxx.json]
// server_ip 使用的自身的server ip(可以多个，可以设计成list)
// use_cache 可选参数，是否读取本地的集群信息
int start(char *server_ip, int use_cache) {
    init_socket();

    if (!fopen("InitTest/node_info.txt", "r")) {
        // 不存在infofile
        // 生成本地infofile
        printf("infofile not exist\n");
        generate_info();
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

            struct2file(local_node, "InitTest/node_info.txt");
        }
    }
    return 0;
}

int main(int argc,char *argv[]) {
    start(argv[1], 0);
    return 0;
}
