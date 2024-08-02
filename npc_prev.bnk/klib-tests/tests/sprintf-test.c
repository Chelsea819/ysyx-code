#include "../include/ktest.h"
#define N 8

int main(){
    int data[N] = {0,126322567,2147483647,-2147483648,-2147483647,252645135,126322567,-1};
    char data_arr[N][15] = {"0\0","126322567\0","2147483647\0","-2147483648\0","-2147483647\0","252645135\0","126322567\0","-1\0"};
    char data_tmp[N][15] = {0};
    for(int i = 0; i < N; i ++){
        sprintf(data_tmp[i],"%d",data[i]);
    }
    for(int i = 0; i < N; i ++){
        assert(strcmp(data_arr[i],data_arr[i]) == 0);
    }
    // for(int i = 0; i < N; i ++){
    //     printf("data_tmp[%d] = %s\n",i,data_tmp[i]);
    // }
    return 0;
}