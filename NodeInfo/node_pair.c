#include "node_pair.h"

#define P NodePair

P create_node_pair(struct Node *fromNode, struct Node *toNode, char *to_ip) {
    P pair;
    pair = (P)malloc(sizeof(P));

    pair->fromNode = fromNode;
    pair->toNode = toNode;
    pair->to_ip = to_ip;

    return pair;
}

W create_width_delay(int delay, double width) {
    W w;
    w = (W)malloc(sizeof(W));

    w->delay = delay;
    w->width = width;

    return w;
}

// int main() {
//     struct Node node1;
//     node1.id = 1;
//     node1.node_count = 100;
//     struct Node node2;
//     node2.id = 2;
//     node2.node_count = 200;

//     P pair = create_node_pair(&node1, &node2);
//     printf("fromNode id: %d, toNode id: %d\n", pair->fromNode->id, pair->toNode->id);
//     printf("fromNode count: %d, toNode count: %d\n", pair->fromNode->node_count, pair->toNode->node_count);
// }