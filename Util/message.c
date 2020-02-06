#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"

int big_buf[FILE_SIZE];

int read_file_to_buf(struct Message *message, char *file_name) {
    FILE *fp = fopen(file_name, "r");
    int newLen = -1;
    if (fp != NULL) {
        newLen = fread(message->buf, sizeof(char), BUF_SIZE, fp);
        if (ferror(fp) != 0) {
            fputs("Error reading file", stderr);
        } else {
            message->buf[newLen++] = '\0';
        }
    }
    message->size = newLen;
    fclose(fp);
    return newLen;
}

int write_buf_to_file(struct Message *message, char *file_name, size_t buf_len) {
    FILE *fp = fopen(file_name, "w+");
    int writeLen;
    if (fp != NULL) {
        writeLen = fwrite(message->buf, sizeof(char), buf_len, fp);
        if (ferror(fp) != 0) {
            fputs("Error writing file", stderr);
        }
    }
    fclose(fp);
    return writeLen;
}

int send_msg_over_socket(struct Message *message, int socket_fd) {
    int temp = htonl(message->size);
    if (send(socket_fd, &temp, sizeof(temp), 0) < 0) {
        printf("send message length failed\n");
        return -1;
    }
    if (send(socket_fd, message->type, 20, 0) < 0) {
        printf("send message type failed\n");
        return -1;
    }
    if (send(socket_fd, message->buf, message->size, 0) < 0) {
        printf("send message buf failed\n");
        return -1;
    }

    printf("Send Message Success, Tpye: %s, Size: %d\n", message->type, message->size);
    return 0;
}


int send_file_over_socket(int socket_fd) {
    int temp = FILE_SIZE;
    char *type = WIDTH_TEST;
    if (send(socket_fd, &temp, sizeof(temp), 0) < 0) {
        printf("send file length failed\n");
        return -1;
    }
    if (send(socket_fd, type, 20, 0) < 0) {
        printf("send message type failed\n");
        return -1;
    }
    if (send(socket_fd, big_buf, FILE_SIZE, 0) < 0) {
        printf("send file failed\n");
        return -1;
    }

    printf("Send File Success\n");
    return 0;
}

// 使用socket接收大文件，用于进行带宽测试
int recv_file_over_socket(int socket_fd) {
    if (recv(socket_fd, big_buf, FILE_SIZE, 0) < 0) {
        printf("receive file failed\n");
        return -1;
    }

    printf("buf: %s\n", big_buf);

    return 0;
}

int recv_msg_over_socket(struct Message *message, int socket_fd) {
    int temp;
    if (recv(socket_fd, &temp, sizeof(temp), 0) < 0) {
        printf("receive message length failed\n");
        return -1;
    }
    char msg_type[20];
    if (recv(socket_fd, msg_type, 20, 0) < 0) {
        printf("receive message type failed\n");
        return -1;
    }
    if (strcmp(msg_type, WIDTH_TEST) == 0) {
        if (recv_file_over_socket(socket_fd) < 0) {
            return -1;
        }
        strcpy(message->buf, WIDTH_TEST_RETURN);
        strcpy(message->type, WIDTH_TEST_RETURN);
        message->size = FILE_SIZE;

        return 0;
    } else {
        int msg_len = ntohl(temp);
        char tempbuf[msg_len];
        if (recv(socket_fd, tempbuf, msg_len, 0) < 0) {
            printf("receive message buf failed\n");
            return -1;
        }
        strcpy(message->buf, tempbuf);
        strcpy(message->type, msg_type);
        message->size = msg_len;
        printf("Recevive Message Success, Type: %s, Size: %d\n", message->type, message->size);
        return 0;
    }

    return 0;
}