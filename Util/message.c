#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "message.h"

int read_file_to_buf(struct Message *message, char *file_name) {
    // message = (struct Message*)malloc(sizeof(struct Message));
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
    FILE *fp = fopen(file_name, "w");
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