#include "node_info.h"

int startsWith(const char *pre, const char *str) {
    size_t lenpre = strlen(pre),
           lenstr = strlen(str);
    return lenstr < lenpre ? 0 : memcmp(pre, str, lenpre) == 0;
}

char *strip(const char *s) {
    char *p = malloc(strlen(s) + 1);
    if(p) {
        char *p2 = p;
        while(*s != '\0') {
            if(*s != '\t' && *s != '\n') {
                *p2++ = *s++;
            } else {
                ++s;
            }
        }
        *p2 = '\0';
    }
    return p;
}

int struct2file(struct Node *node, char *filename) {
    FILE *fp;
    if ((fp = fopen(filename, "w")) == NULL) {
        printf("Open file failed.\n");
        return -1;
    }
    // 遍历node列表，写入
    struct Node *p;
    struct IP *ip;
    for(p = node; p != NULL;p = p->next_node) {
        // 标记开始
        fprintf(fp, "NODE\n");
       
        // 正式写入
        fprintf(fp, "id: %d\n", p->id);
        fprintf(fp, "node_count: %d\n", p->node_count);
        fprintf(fp, "reliable: %d\n", p->reliable);
        fprintf(fp, "ip_list\n");
        // 遍历ip列表
        for (ip = p->ip_list; ip != NULL; ip = ip->next_ip) {
            fprintf(fp, "ip: %s\n", ip->ip);
            fprintf(fp, "ip_type: %s\n", ip->type);
            fprintf(fp, "s_count: %d\n", ip->s_count);
        }
        fprintf(fp, "ip_list_ends\n");
        printf("ip_list_ends\n");
    }
    fprintf(fp, "end_node\n");

    fclose(fp);

    return 0;
}

struct Node* file2struct(char *filename) {
    FILE *fp;
    char *strline;
    char buffer[255];
    struct Node *node_new = NULL, *node_tail = NULL;
    struct Node *header = (struct Node*)malloc(sizeof(struct Node));

    header->id = 0;
    header->next_node = NULL;
    node_tail = header;

    struct IP *ip_new = NULL;
    struct IP *ip_header = NULL;
    fp = fopen(filename, "r+");
    if (fp == NULL) {
        printf("is NULL\n");
    }
    while (fgets(buffer, 255, (FILE*) fp)) {
        strline = &buffer[0];
        strline[strlen(strline) - 1] = 0;
        if (startsWith("NODE", strline)) {
            // 是一个Node的开始
            node_new = (struct Node*)malloc(sizeof(struct Node));
            node_new->next_node = NULL;
            node_tail->next_node = node_new;
            node_tail = node_new;
        } else if (startsWith("id: ", strline)) {
            // Node的id
            //printf("line is: %s, len is: %d\n", strline, strlen(strline));
            strline = strline + 4;
            node_new->id = atoi(strline);
            //printf("id is: %d\n", node_new->id);
        } else if (startsWith("node_count: ", strline)) {
            strline = strline + 12;
            node_new->node_count = atoi(strline);
        } else if (startsWith("reliable: ", strline)) {
            strline = strline + 10;
            node_new->reliable = atoi(strline);
        } else if (startsWith("ip_list_ends", strline)) {
            free(ip_new);
            ip_new = NULL;
            ip_header->next_ip = NULL;
            free(ip_header);
            ip_header = NULL;
        } else if (startsWith("ip: ", strline)) {
            strline = strline + 4;
            char *ip = strip(strline);
            strcpy(ip_new->ip, ip);
        } else if (startsWith("ip_type: ", strline)) {
            strline += 9;
            char *ip_type = strip(strline);
            //printf("this is ip_type: %s\n", ip_type);
            strcpy(ip_new->type, ip_type);
        } else if (startsWith("s_count: ", strline)) {
            strline += 9;
            ip_new->s_count = atoi(strline);
            // printf("this is s_count: %d\n", atoi(strline));
            // 创建一个新的ip struct
            ip_header = ip_header->next_ip;
            struct IP *ip_tmp = (struct IP*)malloc(sizeof(struct IP));
            ip_tmp->next_ip = NULL;
            ip_new->next_ip = ip_tmp;
            ip_new = ip_tmp;
        } else if (startsWith("ip_list", strline)) {
            // ip列表的开始
            ip_new = (struct IP*)malloc(sizeof(struct IP));
            ip_header = (struct IP*)malloc(sizeof(struct IP));
            ip_header->next_ip = ip_new;
            ip_new->next_ip = NULL;
            node_new->ip_list = ip_new;
        } else if (startsWith("end_node", strline)) {
            // do nothing
            break;
        }
    }

    return header->next_node;
}

// 比较两个node是否完全相同
int node_is_identical(struct Node *node1, struct Node *node2) {
    struct IP *ip1 = node1->ip_list;
    struct IP *ip2 = node2->ip_list;

    if (node1->id != node2->id) {
        return 0;
    }
    
    if (node1->node_count != node2->node_count) {
        return 0;
    }

    while (1)
    {
        if (ip1 == NULL && ip2 == NULL) {
            return 1;
        }
        if ((ip1 == NULL && ip2 != NULL) || (ip1 != NULL && ip2 == NULL)) {
            return 0;
        }
        if ((!strcmp(ip1->ip, ip2->ip)) || (!strcmp(ip1->type, ip2->type)) || (ip1->s_count != ip2->s_count)) {
            return 0;
        }
        ip1 = ip1->next_ip;
        ip2 = ip2->next_ip;
    }
    
}

int ip_list_merge(struct IP *ip1, struct IP *ip2) {
    struct IP *ip_list_tail = NULL;
    for (ip_list_tail = ip1;ip_list_tail->next_ip != NULL;ip_list_tail = ip_list_tail->next_ip) {
        // 遍历到最后一个IP
    }

    struct IP *ip_ptr = NULL;
    for (ip_ptr = ip2;ip_ptr != NULL; ip_ptr = ip_ptr->next_ip) {
        struct IP *ip_tmp = NULL;
        int flag = 1;
        for (ip_tmp = ip1;ip_tmp != NULL;ip_tmp = ip_tmp->next_ip) {
            if (strcmp(ip_tmp->ip, ip_ptr->ip) == 0) {
                // 相同ip，根据s_count合并
                flag = 0;
                if(ip_ptr->s_count > ip_tmp->s_count) {
                    strcpy(ip_tmp->ip, ip_ptr->ip);
                    strcpy(ip_tmp->type, ip_ptr->type);
                    ip_tmp->s_count = ip_ptr->s_count;
                } else if (ip_ptr->s_count == ip_tmp->s_count) {
                    if ((strcmp(ip_ptr->ip, ip_tmp->ip) != 0) || (strcmp(ip_ptr->type, ip_tmp->type) != 0)) {
                        // 强冲突
                        return -1;
                    }
                }
            }
        }
        // 不存在的ip，直接接到链表尾部
        if (flag) {
            ip_list_tail->next_ip = ip_ptr;
            ip_list_tail = ip_list_tail->next_ip;
        }
    }

    return 1;
}

// 集群信息合并，结果统一放在第一个node中
// 如果发生强冲突 返回 -1
int node_merge(struct Node *node1, struct Node* node2) {
    struct Node *node_tail = NULL;

    for (node_tail = node1;node_tail->next_node != NULL;node_tail = node_tail->next_node) {
        // 遍历到node1的最后一个节点
    }

    struct Node *node_ptr = NULL;
    for (node_ptr = node2;node_ptr != NULL;node_ptr = node_ptr->next_node) {
        struct Node *node_tmp = NULL;
        int flag = 1; // 是否需要合并到链表尾部
        for (node_tmp = node1;node_tmp != NULL; node_tmp = node_tmp->next_node) {
            if (node_tmp->id != node_ptr->id) {
                continue;
            } else {
                flag = 0;
            }
            if (node_ptr->node_count > node_tmp->node_count) {
                node_tmp->node_count = node_ptr->node_count;
                node_tmp->ip_list = node_ptr->ip_list;
                node_tmp->reliable = node_ptr->reliable;
            } else if (node_ptr->node_count == node_tmp->node_count) {
                if (ip_list_merge(node_tmp->ip_list, node_ptr->ip_list) == -1) {
                    // 强冲突
                    printf("need recheck\n");
                    return -1;
                }
            }
        }

        // 没有相同id，直接合并到链表尾部
        if (flag) {
            node_tail->next_node = node_ptr;
            node_tail = node_tail->next_node;
        }
    }
    return 1;
}

// int main() {
//     struct Node node;
//     bzero(&node, sizeof(node));
//     node.id = 1;
//     node.node_count = 1;
//     struct IP ip1;
//     bzero(&ip1, sizeof(ip1));
//     strcpy(ip1.ip, "127.0.0.1");
//     strcpy(ip1.type, "clientIP");
//     ip1.s_count = 1;
//     node.ip_list = &ip1;

//     // struct Node node2;
//     // bzero(&node2, sizeof(node2));
//     // node2.id = 1;
//     // node2.node_count = 1;
//     // struct IP ip3;
//     // bzero(&ip3, sizeof(ip3));
//     // strcpy(ip3.ip, "127.0.0.1");
//     // strcpy(ip3.type, "serverIP");
//     // ip3.s_count = 2;
//     // node2.ip_list = &ip3;

//     // int result = node_merge(&node, &node2);

//     // struct Node *p = NULL;
//     // for (p = &node; p != NULL; p = p->next_node) {
//     //     printf("node id: %d, node_count: %d\n", p->id, p->node_count);
//     //     struct IP *tmp = NULL;
//     //     for (tmp = p->ip_list;tmp != NULL;tmp = tmp->next_ip) {
//     //         printf("ip: %s, type: %s, s_count: %d\n", tmp->ip, tmp->type, tmp->s_count);
//     //     }
//     // }

    

//     struct2file(&node, "node_info.txt");
//     struct Node *nodefromfile = file2struct("node_info.txt");
    
//     struct Node *p;
//     struct IP *ip;
//     for(p = nodefromfile; p != NULL;p = p->next_node) {
//         // 标记开始
//         printf("Node start\n");
//         // 正式写入
//         printf("id: %d\n", p->id);
//         printf("node_count: %d\n", p->node_count);

//         printf("ip_list\n");
//         // 遍历ip列表
//         for (ip = p->ip_list; ip != NULL; ip = ip->next_ip) {
//             printf("ip: %s\n", ip->ip);
//             printf("ip_type: %s\n", ip->type);
//             printf("s_count: %d\n", ip->s_count);
//         }
//         printf("ip_list_end\n");
//     }

// }