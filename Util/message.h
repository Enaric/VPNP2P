
#define BUF_SIZE 1024 * 64
#define TYPE_SIZE 20
#define FILE_SIZE 1<<19 // 0.5MB，因为socket package大小限制暂时无法使用1MB 
#define REGULAR_UPDATE "regular_update"
#define REGULAR_UPDATE_RETURN "regular_update_return"
#define WIDTH_TEST "width_test"
#define WIDTH_TEST_RETURN "width_test_return"

struct Message {
    char type[TYPE_SIZE];
    char buf[BUF_SIZE];
    int size;
};

// 将file_name对应文件的内容读入到message结构体的buf中， 以字符串形式存放
int read_file_to_buf(struct Message *message, char *file_name);

// 将message结构体中的buf写入到file_name对应的文件中，buf的长度为buf_len
int write_buf_to_file(struct Message *message, char *file_name, size_t buf_len);

// 使用socket发送message到另一台主机，socket_fd为对应连接的socket file descriptor
int send_msg_over_socket(struct Message *message, int socket_fd);

// 用socket发送大文件，用于带宽测试
int send_file_over_socket(int socket_fd);

// 使用socket接收其他主机发来的message，socket_fd为对应连接的socket file descriptor
int recv_msg_over_socket(struct Message *message, int socket_fd);