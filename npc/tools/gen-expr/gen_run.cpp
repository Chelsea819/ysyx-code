#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <exception>
int gen_run(char *buf, int *result){
    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(buf, fp);
    fclose(fp);
    int ret = 0;

    //执行shell命令
    try{
        ret = system("gcc /tmp/.code.c -o /tmp/.expr");     
    }
    catch(std::exception& e){
        printf("Catch exception!");
        return -1;
    }
    if (ret != 0) return -1;
    //使用 popen() 打开一个管道,与 /tmp/.expr 进程的输出交互
    //输出将存入fp
    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    ret = fscanf(fp, "%d", result);
    pclose(fp);
    return 0;
}
