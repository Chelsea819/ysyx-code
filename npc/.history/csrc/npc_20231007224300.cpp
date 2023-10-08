#include <getopt.h>
#include "npc.h"
#include "common.h"
#include "paddr.h"

void init_mem_npc();
void init_isa();
void init_sdb();
void init_rand();
void sdb_set_batch_mode();

static char *img_file = NULL;

static void welcome() {
  printf("Welcome to NPC!\n");
  printf("For help, type \"help\"\n");
}

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

  printf("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  //fread()可以高效地从文件流中读取大块的二进制数据,放入指定的内存缓冲区中
  assert(ret == 1);

  fclose(fp);
  return size;
}


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
      case 'l': break;
      case 'd': break;
      case 'f': break;
      case 1: img_file = optarg; return 0;
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

    //init memory
    init_mem_npc();

    //load img to memory
    init_isa();

    //load certain program to memory
    long img_size = load_img();

    /* Initialize the simple debugger.初始化简单调试器 */
    init_sdb();

    welcome();
}