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
        fprintf(fp, "end_node\n");
    }
   

    fclose(fp);

    return 0;
}

struct Node* file2struct(char *filename) {
    FILE *fp;
    char *strline;
    char buffer[255];
    // todo 不用 node_tail
    struct Node *node_new = (struct Node*)malloc(sizeof(struct Node));
    struct Node *header = (struct Node*)malloc(sizeof(struct Node));

    header->id = 0;
    header->next_node = node_new;
    struct Node *prev = header;

    struct IP *ip_new = (struct IP*)malloc(sizeof(struct IP));
    struct IP *ip_header = (struct IP*)malloc(sizeof(struct IP));
    fp = fopen(filename, "r+");
    if (fp == NULL) {
        printf("is NULL\n");
    }
    while (fgets(buffer, 255, (FILE*) fp)) {
        strline = &buffer[0];
        strline[strlen(strline) - 1] = 0;
        // printf("strline: %s\n", strline);
        if (startsWith("NODE", strline)) {
            // 是一个Node的开始
            // node_new = (struct Node*)malloc(sizeof(struct Node));
            // node_new->next_node = NULL;
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
            ip_header->next_ip = NULL;
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
            ip_new = ip_new->next_ip;
        } else if (startsWith("ip_list", strline)) {
            // ip列表的开始
            ip_new = (struct IP*)malloc(sizeof(struct IP));
            ip_header = (struct IP*)malloc(sizeof(struct IP));
            ip_header->next_ip = ip_new;
            ip_new->next_ip = NULL;
            node_new->ip_list = ip_new;
        } else if (startsWith("end_node", strline)) {
            struct Node *node_next = (struct Node*)malloc(sizeof(struct Node));
            node_new->next_node = node_next;
            node_new = node_new->next_node;
            prev = prev->next_node;
        }
    }
    prev->next_node = NULL;
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
int node_merge(struct Node* node1, struct Node* node2) {
    // printf("node1: \n");
    // print_node(node1);
    // printf("node2: \n");
    // print_node(node2);
    struct Node *ptr1;
    struct Node *ptr2;
    struct Node *remain = (struct Node*)malloc(sizeof(struct Node));
    struct Node *p = (struct Node*)malloc(sizeof(struct Node));
    struct Node *prev;
    prev = remain;
    remain->next_node = p;
    
    for (ptr2 = node2; ptr2 != NULL; ptr2 = ptr2->next_node) {
        int flag = 1;
        for (ptr1 = node1; ptr1 != NULL; ptr1 = ptr1->next_node) {
            if (ptr1->id == ptr2->id) {
                flag = 0;
                if (ptr1->node_count < ptr2->node_count) {
                    ptr1->node_count = ptr2->node_count;
                    ptr1->reliable = ptr2->reliable;
                    ptr1->ip_list = ptr2->ip_list;
                }
                // todo node_count相等
            }
        }
        if (flag) {
            p->id = ptr2->id;
            p->node_count = ptr2->node_count;
            p->reliable = ptr2->reliable;
            p->ip_list = ptr2->ip_list;
            p = p->next_node;
            prev = prev->next_node;
        }
    }
    for (ptr1 = node1; ptr1->next_node != NULL; ptr1 = ptr1->next_node) {

    }
    prev->next_node = NULL;
    ptr1->next_node = remain->next_node;
}

int contain_node(struct Node *list, struct Node *node) {
    struct Node *tmp;
    for (tmp = list; tmp != NULL; tmp = tmp->next_node) {
        if (tmp->id == node->id) {
            return 1;
        }
    }

    return 0;
}

void print_node(struct Node *node) {
    struct Node *ptr = NULL;
    struct IP *ip = NULL;
    for (ptr = node;ptr != NULL;ptr = ptr->next_node) {
        printf("node id: %d, node count: %d\n", ptr->id, ptr->node_count);
        printf("node ip list: \n");
        for (ip = ptr->ip_list;ip != NULL;ip = ip->next_ip) {
            printf("ip: %s, type: %s, s_count: %d\n", ip->ip, ip->type, ip->s_count);
        }
    }
}

struct Node* convert_buf_to_node(char *buf) {
    char dst[50][80];
    int cnt = split(dst, buf, "\n");

    struct Node *node_new = (struct Node*)malloc(sizeof(struct Node));
    struct Node *header = (struct Node*)malloc(sizeof(struct Node));

    header->id = 0;
    header->next_node = node_new;
    struct Node *prev = header;

    struct IP *ip_new = NULL;
    struct IP *ip_header = NULL;

    for (int i = 0;i < cnt;i++) {
        if (startsWith("NODE", dst[i])) {
            // 是一个Node的开始
        } else if (startsWith("id:", dst[i])) {
            char id[10];
            memcpy(id, dst[i] + 4, 10);
            node_new->id = atoi(id);
        } else if (startsWith("node_count:", dst[i])) {
            char node_count[10];
            memcpy(node_count, dst[i] + 12, 10);
            node_new->node_count = atoi(node_count);
        } else if (startsWith("reliable:", dst[i])) {
            char reliable[10];
            memcpy(reliable, dst[i] + 10, 10);
            node_new->reliable = atoi(reliable);
        } else if (startsWith("ip_list_ends", dst[i])) {
            ip_header->next_ip = NULL;
        } else if (startsWith("ip:", dst[i])) {
            char ip[20];
            memcpy(ip, dst[i] + 4, 20);
            strcpy(ip_new->ip, ip);
        } else if (startsWith("ip_type:", dst[i])) {
            char ip_type[10];
            memcpy(ip_type, dst[i] + 9, 10);
            strcpy(ip_new->type, ip_type);
        } else if (startsWith("s_count:", dst[i])) {
            char s_count[10];
            memcpy(s_count, dst[i] + 9, 10);
            ip_new->s_count = atoi(s_count);

            ip_header = ip_header->next_ip;
            struct IP *ip_tmp = (struct IP*)malloc(sizeof(struct IP));
            ip_tmp->next_ip = NULL;
            ip_new->next_ip = ip_tmp;
            ip_new = ip_tmp;
        } else if (startsWith("ip_list", dst[i])) {
            // ip列表的开始
            ip_new = (struct IP*)malloc(sizeof(struct IP));
            ip_header = (struct IP*)malloc(sizeof(struct IP));
            ip_header->next_ip = ip_new;
            ip_new->next_ip = NULL;
            node_new->ip_list = ip_new;
        } else if (startsWith("end_node", dst[i])) {
            struct Node *node_next = (struct Node*)malloc(sizeof(struct Node));
            node_new->next_node = node_next;
            node_new = node_new->next_node;
            prev = prev->next_node;
        }
    }
    prev->next_node = NULL;
    return header->next_node;
}

int convert_node_to_buf(struct Node *node, char buf[2048]) {
     // 遍历node列表，写入
    struct Node *p;
    struct IP *ip;
    int offset = 0;
    // static char buf[2048];
    for(p = node; p != NULL;p = p->next_node) {
        // 标记开始
        offset += sprintf(buf + offset, "NODE\n");
        
        // 正式写入
        offset += sprintf(buf + offset, "id: %d\n", p->id);
        offset += sprintf(buf + offset, "node_count: %d\n", p->node_count);
        offset += sprintf(buf + offset, "reliable: %d\n", p->reliable);
        offset += sprintf(buf + offset, "ip_list\n");
        // 遍历ip列表
        for (ip = p->ip_list; ip != NULL; ip = ip->next_ip) {
            // printf("ip: %s\n", ip->ip);
            // printf("ip_type: %s\n", ip->type);
            // printf("s_count: %d\n", ip->s_count);
            offset += sprintf(buf + offset, "ip: %s\n", ip->ip);
            offset += sprintf(buf + offset, "ip_type: %s\n", ip->type);
            offset += sprintf(buf + offset, "s_count: %d\n", ip->s_count);
        }
        offset += sprintf(buf + offset, "ip_list_ends\n");
        offset += sprintf(buf + offset, "end_node\n");
    }
    return offset;
}

int split(char dst[][80], char *str, const char *spl) {
    int n = 0;
    char *result = NULL;
    result = strtok(str, spl);
    while( result != NULL )
    {
        strcpy(dst[n++], result);
        result = strtok(NULL, spl);
    }
    return n;
}

int main() {
    struct Node *node1 = file2struct("node_info.txt");
    struct Node *node2 = file2struct("test_info.txt");
    //print_node(node1);
    node_merge(node1, node2);
    print_node(node1);
}
