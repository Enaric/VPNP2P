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
#include "../NodeInfo/node_info.h"

#define FILENAME "node_info.txt"

// int Listen_PORT;
struct Node *local_node;             // 本地节点

// todo 写到util中实现复用
int generate_node_info(char *server_ip) {
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
    // 增加server ip信息到节点
    struct IP *ip_server = (struct IP *)malloc(sizeof(struct IP));
    strcpy(ip_server->type, "serverIP");
    strcpy(ip_server->ip, server_ip);
    ip_server->s_count = 0;
    ip->next_ip = ip_server;
    ip = ip->next_ip;
    // 将生成的节点信息写到文件
    struct2file(&tmp_node, FILENAME);

    if (ip != NULL) { free(ip); ip = NULL; }
    if (local_ip_list != NULL) { free(local_ip_list); local_ip_list = NULL; }
    return 0;
}

// 初始搭建，第一个Node必须指定一个serverIP
//./net start serverIP1 [, serverIP2, infofile=xxx.json]
// server_ip 使用的自身的server ip(可以多个，可以设计成list)
// use_cache 可选参数，是否读取本地的集群信息
int start(char *server_ip, int use_cache) {

    if (!fopen(FILENAME, "r")) {
        // 不存在infofile
        // 生成本地infofile
        printf("infofile not exist\n");
        generate_node_info(server_ip);
    } else {
        printf("infofile exist\n");
    }
    
    if (use_cache) {
       // todo
    }
    return 0;
}

// int main(int argc,char *argv[]) {
//     start(argv[1], 0);
//     return 0;
// }
