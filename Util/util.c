#include "util.h"

int is_intranet(char *ip) {
    return 1;
}

// 获取本机ip
char** get_local_ip() {
    int fd, intrface, retn = 0;
    struct ifreq buf[INET_ADDRSTRLEN];
    struct ifconf ifc;
    char **local_ip_list = malloc(IP_LIST_SIZE * sizeof(char*));
    int length = 0;

    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0) {
        ifc.ifc_len = sizeof(buf);
        ifc.ifc_buf = (caddr_t) buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)) {
            intrface = ifc.ifc_len / sizeof(struct ifreq);
            while (intrface-- > 0) {
                if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface]))) {
                    char *ip = (inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
                    //printf("IP:%s\n", ip);
                    local_ip_list[length] = malloc(IP_SIZE * sizeof(char));
                    strcpy(local_ip_list[length], ip);
                    length++;
                }
            }
            
        }
    }

    close(fd);
    return local_ip_list;
}

int ip_list_contains(char **ip_list, int length, char *ip) {
    for (int i = 0;i < length;i++) {
        if (strcmp(ip_list[i], ip) == 0) {
            return 1;
        }
    }

    return 0;
}

// 判断node中的ip list是否和当前自身的ip地址全部对应
int ip_match(char **ip_list, int length) {
    char **local_ip_list = get_local_ip();
    int local_ip_list_length = 0;
    for (int i = 0;i < IP_LIST_SIZE;i++) {
        if (local_ip_list[i] == NULL) {
            break;
        }
        local_ip_list_length++;
    }

    if (length != local_ip_list_length) {
        return 0;
    }

    for (int i = 0;i < length;i++) {
        // 存在不对应的ip
        if(! ip_list_contains(local_ip_list, local_ip_list_length, ip_list[i])) {
            return 0;
        }
    }

    return 1;
}

// todo 用snowflake算法实现
int generate_id() {
    int i;
    srand((unsigned)time(NULL));
    return rand() % 10000 + 1;
}

int init_socket() {
    bzero(&client_addr, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htons(INADDR_ANY);
    client_addr.sin_port = htons(0);
    return 0;
}



int connect_to_server(int client_socket_fd, char *target_ip, int port) {
    init_socket();

    bzero(&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    if(inet_pton(AF_INET, target_ip, &server_addr.sin_addr) == 0)
    {
        perror("Server IP Address Error:");
        return -1;
    }

    server_addr.sin_port = htons(port);
    socklen_t server_addr_length = sizeof(server_addr);

    // 向服务器发起连接
    if(connect(client_socket_fd, (struct sockaddr*)&server_addr, server_addr_length) < 0)
    {
        perror("Can Not Connect To Server IP:");
        return -1;
    }
    
    return 0;
}