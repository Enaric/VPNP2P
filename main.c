#include <stdio.h>

int join(char *, int);

int main(int argc,char *argv[]) {
    //join("47.99.47.195", 0);
    if (argc != 2){
        printf("请检查参数个数");
        return 0;
    }
    join(argv[1], 0);
    return 0;
}