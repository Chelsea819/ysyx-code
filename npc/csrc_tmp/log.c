#include "common.h"

FILE *log_fp = NULL;
static char *log_file = NULL;
#define CONFIG_TRACE_START 0
#define CONFIG_TRACE_END 10000

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    //Assert宏检查文件打开是否成功,如果失败则打印错误信息并退出
    
    log_fp = fp;
    //如果文件打开成功,则将log_fp指向这个文件
  }
  // printf("log_file:%s\n",log_file);
  // printf("log_fp:%s\n",log_fp);
  Log("Log is written to %s", log_file ? log_file : "stdout");
  //调用Log函数,打印日志初始化信息
  //如果log_file为空,则日志输出到stdout
  //如果指定了文件,则日志输出到该文件
}

bool log_enable() {
  return MUXDEF(CONFIG_TRACE, (g_nr_guest_inst >= CONFIG_TRACE_START) &&
         (g_nr_guest_inst <= CONFIG_TRACE_END), false);
}