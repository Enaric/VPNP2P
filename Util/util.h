#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close
#include <time.h>

#define IP_SIZE     16
#define IP_LIST_SIZE 10

// 判断是否为内网ip
int is_intranet(char *ip);

// 获取本机ip
char** get_local_ip();

int ip_list_contains(char **ip_list, int length, char *ip);

// 判断node中的ip list是否和当前自身的ip地址全部对应
int ip_match(char **ip_list, int length);

int generate_id();