#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h> // for open
#include <unistd.h> // for close

struct IP {
    char ip[20];    //ip
    char type[20];        //unknownIP/clientIP/serverIP
    int s_count;
    struct IP *next_ip;
};

struct Node {//node
    int id;
    int node_count;
    struct IP *ipList;
    struct Node *next_node;
};

int get_local_ip(char *ip){
    int fd, intrface, retn = 0;
    struct ifreq buf[INET_ADDRSTRLEN];
    struct ifconf ifc;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) >= 0){
        ifc.ifc_len = sizeof(buf);
        // caddr_t,Linux内核源码里定义的：typedef void *caddr_t；
        ifc.ifc_buf = (caddr_t)buf;
        if (!ioctl(fd, SIOCGIFCONF, (char *)&ifc)){
            intrface = ifc.ifc_len/sizeof(struct ifreq);
            while (intrface-- > 0){
                if (!(ioctl(fd, SIOCGIFADDR, (char *)&buf[intrface]))){
                    ip=(inet_ntoa(((struct sockaddr_in*)(&buf[intrface].ifr_addr))->sin_addr));
                    printf("IP:%s\n", ip);
                }
            }
        }
        close(fd);
    }
    return 0;
}

int main(){
    char ip[64];
    memset(ip, 0, sizeof(ip));
    get_local_ip(ip);
    return 0;
}
