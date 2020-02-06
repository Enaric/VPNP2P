#include "node_info.h"

#define P NodePair
#define W WidthAndDelay
typedef struct P *P;
typedef struct W *W;

// 表示fromNode和toNode节点的to_ip的关系
// NodeA --> NodeB:IP1
struct P {
    struct Node *fromNode;
    struct Node *toNode;
    char *to_ip;
};

// 表示节点间带宽和时延信息
struct W {
    int delay; // 节点间的时延，单位为 ms
    double width; // 节点间的带宽，单位为 kb/s
};

P create_node_pair(struct Node *fromNode, struct Node *toNode, char *to_ip);

W create_width_delay(int delay, double width);