#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE 1024 // 每行最大字节
#define RELIABLE 1
#define UNRELIABLE 0

struct Node *local_node; // 本地集群信息 todo 线程安全问题
int local_node_id; // 标识本地节点id

struct IP {
    char ip[20];
    char type[20];  //unknownIP / clientIP / serverIP
    int s_count;
    int validity; // ip信息有效性，1代表可信
    struct IP *next_ip;
};

struct Node {
    int id;
    int node_count;
    int reliable; // 1表示可靠，0表示不可靠
    struct IP *ip_list;
    struct Node *next_node;
};

int startsWith(const char *pre, const char *str);

char *strip(const char *s);

int struct2file(struct Node *node, char *filename);

struct Node* file2struct(char *filename);

// 比较两个node是否完全相同
int node_is_identical(struct Node *node1, struct Node *node2);

int ip_list_merge(struct IP *ip1, struct IP *ip2);

// 集群信息合并，结果统一放在第一个node中
// 如果发生强冲突 返回 -1
int node_merge(struct Node *node1, struct Node* node2);

// 打印node信息
void print_node(struct Node *node);

// 将buf转换成node信息，保存在node指针中
struct Node* convert_buf_to_node(char *buf);

// 将node信息写入到buf中
int convert_node_to_buf(struct Node *node, char buf[2048]);

int split(char dst[][80], char *str, const char *spl);

struct Node* generate_local_node();

int ip_list_length(struct IP *ip_list);

// 判断一个ip列表中是否存在某个ip
int contain_ip(struct IP *ip_list, char *ip);

// 判断两个ip列表是否完全一致
int ip_list_identical(struct IP *ip_list1, struct IP *ip_list2);
