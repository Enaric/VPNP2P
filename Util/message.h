#include <stdio.h>
#include <string.h>
#define BUF_SIZE 2048
#define TYPE_SIZE 20

struct Message {
    char type[TYPE_SIZE];
    char buf[BUF_SIZE];
    int size;
};

int read_file_to_buf(struct Message *message, char *file_name);

int write_buf_to_file(struct Message *message, char *file_name, size_t buf_len);

int send_msg_over_socket(struct Message *message, int socket_fd);

int recv_msg_over_socket(struct Message *message, int socket_fd);