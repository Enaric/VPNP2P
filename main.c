#include <stdio.h>
#include <string.h>

int join(char *, int);
int start(char *, int);

int main(int argc,char *argv[]) {
    //join("47.99.47.195", 0);
    if (argc < 2) {
        printf("请检查参数个数");
        return 0;
    }
    if (strcmp(argv[1], "start") == 0) {
        // 生成本地节点信息
        start(argv[2], 0);
    } else if (strcmp(argv[1], "join") == 0) {
        if (argc == 3 && strcmp(argv[2], "-f") == 0) {
            join(argv[2], 1);
        } else {
            join(argv[2], 0);
        } 
    }
    return 0;
}