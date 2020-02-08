#define PORT 3389
#define KB 1<<10

char buf[64 * KB]; // 全局变量数组，用于带宽测试

void local_ip_refresh();

void router_refresh(struct Node *fromNode, struct Node *toNode);
