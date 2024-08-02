/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NPC is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>
#include <paddr.h>

typedef struct ftrace_file{
  char *name;
  int NO;
  struct ftrace_file *next;
} Ftrace_file;

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_isa();
int init_ftrace(Ftrace_file *file_header, Ftrace_file *file_cur);
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NPC!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
//   printf("For help, type \"help\"\n");
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();



static char *log_file = NULL;
// static int flag = 0;
static char *ftrace_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;



#define FILE_NUM 5
Ftrace_file *file_header = NULL;
Ftrace_file *file_pool = NULL;
Ftrace_file *file_cur = NULL;

void creat_ftraceIndex(char *filename){
  printf("filename = %s\n",filename);
  if(file_header == NULL){
    file_pool = (Ftrace_file *)malloc(FILE_NUM * sizeof(Ftrace_file));
    for(int i = 0; i < FILE_NUM; i ++){
      file_pool[i].NO = i;
      file_pool[i].next = (i == FILE_NUM - 1 ? NULL : &file_pool[i + 1]);
    }
    file_header = file_pool;
    file_cur = file_pool;
  }
  file_cur->name = filename;
  file_cur = file_cur->next;
}

//返回加载的镜像文件大小
static long load_img() {
  if (img_file == NULL) {
    printf("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
    //如果img_file为NULL,说明没有指定文件,则使用默认内置的镜像,返回其大小4096
  }

  FILE *fp = fopen(img_file, "rb");
  assert(fp);

  fseek(fp, 0, SEEK_END);
  //ftell()可以获取文件当前的读写位置偏移量
  long size = ftell(fp);

  printf("The image is %s!\nThe size = %ld!\n", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
//   printf("inst = 0x%08x\n",*(uint32_t*)(guest_to_host_npc(RESET_VECTOR)));
  //fread()可以高效地从文件流中读取大块的二进制数据,放入指定的内存缓冲区中
  assert(ret == 1);

  fclose(fp);
  return size;
}

//解析命令行参数
static int parseArgs(int argc, char *argv[]) {
  const struct option table[] = {
    {"batch"    , no_argument      , NULL, 'b'},
    {"log"      , required_argument, NULL, 'l'},
    {"diff"     , required_argument, NULL, 'd'},
    {"port"     , required_argument, NULL, 'p'},
    {"ftrace"   , required_argument, NULL, 'f'},
    {"help"     , no_argument      , NULL, 'h'},
    {0          , 0                , NULL,  0 },
  };
  int o;
  while ( (o = getopt_long(argc, argv, "-bhl:d:p:f:", table, NULL)) != -1) {
    //参数个数 参数数组 短选项列表 长选项列表 处理长选项时返回选项的索引
    switch (o) {
      case 'b': sdb_set_batch_mode(); break; 
      case 'p': break;
      case 'l': log_file = optarg; printf("oparg = %s\n",optarg); break;
      case 'd': diff_so_file = optarg; break;
      case 'f': ftrace_file = optarg; break;
      case 1: img_file = optarg; printf("img_file oparg = %s\n",optarg); return 0;
      default:
        printf("Usage: %s [OPTION...] IMAGE [args]\n\n", argv[0]);
        printf("\t-b,--batch              run with batch mode\n");
        printf("\t-l,--log=FILE           output log to FILE\n");
        printf("\t-f,--ftrace=FILE        get elf from FILE\n");
        printf("\t-d,--diff=REF_SO        run DiffTest with reference REF_SO\n");
        printf("\t-p,--port=PORT          run DiffTest with port PORT\n");
        printf("\n");
        exit(0);
    }
  }
  return 0;
}

void init_npc(int argc,char *argv[]){

    //parse shell arguments
    parseArgs(argc, argv);

    /* Set random time seed. */
    init_rand();

    /* Open the log file. */
    init_log(log_file);

    //init memory
    init_mem();

    /* Initialize devices. */
    // IFDEF(CONFIG_DEVICE, init_device());

    //load img to memory
    init_isa();

    //load certain program to memory
    long img_size = load_img();

#ifdef CONFIG_DIFFTEST
    /* Initialize differential testing. */
    init_difftest(diff_so_file, img_size, difftest_port);
#endif

    /* Initialize the simple debugger.初始化简单调试器 */
    init_sdb();

    #ifdef CONFIG_FTRACE
    init_ftrace(ftrace_file);
    #endif

    #ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32e"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif
    welcome();
}
#else // CONFIG_TARGET_AM
static long load_img() {
  extern char bin_start, bin_end;
  size_t size = &bin_end - &bin_start;
  Log("img size = %ld", size);
  memcpy(guest_to_host(RESET_VECTOR), &bin_start, size);
  return size;
}

void am_init_monitor() {
  init_rand();
  init_mem();
  init_isa();
  load_img();
  IFDEF(CONFIG_DEVICE, init_device());
  welcome();
}
#endif
