#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_LINE 1024 // 每行最大字节
#define RELIABLE 1
#define UNRELIABLE 0

struct IP {
    char ip[20];
    char type[20];  //unknownIP / clientIP / serverIP
    int s_count;
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