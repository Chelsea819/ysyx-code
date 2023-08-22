/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
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
#include <memory/paddr.h>
#include <elf.h>

void init_rand();
void init_log(const char *log_file);
void init_mem();
void init_difftest(char *ref_so_file, long img_size, int port);
void init_device();
void init_sdb();
void init_disasm(const char *triple);

static void welcome() {
  Log("Trace: %s", MUXDEF(CONFIG_TRACE, ANSI_FMT("ON", ANSI_FG_GREEN), ANSI_FMT("OFF", ANSI_FG_RED)));
  IFDEF(CONFIG_TRACE, Log("If trace is enabled, a log file will be generated "
        "to record the trace. This may lead to a large log file. "
        "If it is not necessary, you can disable it in menuconfig"));
  Log("Build time: %s, %s", __TIME__, __DATE__);
  printf("Welcome to %s-NEMU!\n", ANSI_FMT(str(__GUEST_ISA__), ANSI_FG_YELLOW ANSI_BG_RED));
  printf("For help, type \"help\"\n");
  //Log("Exercise: Please remove me in the source code and compile NEMU again.");
  //assert(0);
}

#ifndef CONFIG_TARGET_AM
#include <getopt.h>

void sdb_set_batch_mode();

static char *log_file = NULL;
static char *ftrace_file = NULL;
static char *diff_so_file = NULL;
static char *img_file = NULL;
static int difftest_port = 1234;

//返回加载的镜像文件大小
static long load_img() {
  if (img_file == NULL) {
    Log("No image is given. Use the default build-in image.");
    return 4096; // built-in image size
    //如果img_file为NULL,说明没有指定文件,则使用默认内置的镜像,返回其大小4096
  }

  FILE *fp = fopen(img_file, "rb");
  Assert(fp, "Can not open '%s'", img_file);

  fseek(fp, 0, SEEK_END);
  //ftell()可以获取文件当前的读写位置偏移量
  long size = ftell(fp);

  Log("The image is %s, size = %ld", img_file, size);

  fseek(fp, 0, SEEK_SET);
  int ret = fread(guest_to_host(RESET_VECTOR), size, 1, fp);
  //fread()可以高效地从文件流中读取大块的二进制数据,放入指定的内存缓冲区中
  assert(ret == 1);

  fclose(fp);
  return size;
}

FILE *ftrace_fp = NULL;

  Elf32_Ehdr Elf_header;
  Elf32_Shdr Elf_sec;
  Elf32_Off sym_off;
  Elf32_Off str_off;
  Elf32_Sym  Elf_sym;
  Elf32_Xword str_size;
  char *strtab = NULL;

static int init_ftrace(){
  FILE *fp = NULL;
  
  //检查文件是否能正常读取
  Assert(ftrace_file, "ftrace_file is NULL!\n");

  fp = fopen(ftrace_file,"r");
  Assert(fp, "Can not open '%s'",ftrace_file);

  ftrace_fp = fp;
  
  //读取ELF header
  int ret = fread(&Elf_header,sizeof(Elf32_Ehdr),1,ftrace_fp);
  if (ret != 1) {
    perror("Error reading from file");
  }
  if(Elf_header.e_ident[0] != '\x7f' || memcmp(&(Elf_header.e_ident[1]),"ELF",3) != 0){
    Assert(0,"Not an ELF file!\n");
  }

  Assert(Elf_header.e_ident[EI_CLASS] == ELFCLASS32,"Not a 32-bit ELF file\n");
  Assert(Elf_header.e_type == ET_EXEC,"Not an exec file\n");

  //移到.strtab的位置，并进行读取
  fseek(ftrace_fp,Elf_header.e_shoff + Elf_header.e_shentsize * (Elf_header.e_shstrndx - 1),SEEK_SET);
  ret = fread(&Elf_sec,Elf_header.e_shentsize,1,ftrace_fp);
    if (ret != 1) {
      perror("Error reading from file");
    }
  str_off = Elf_sec.sh_offset;
  str_size = Elf_sec.sh_size;
  strtab = malloc(str_size);
  
  fseek(ftrace_fp,str_off,SEEK_SET);
  ret = fread(strtab,str_size,1,ftrace_fp);
  if (ret != 1) {
    printf("ret = %d\n",ret);
    perror("Error reading from file");
  }
  
  // printf("Elf_header.e_shstrndx = %d\n",Elf_header.e_shstrndx);
  // printf("Elf_header.e_shoff = %d\n",Elf_header.e_shoff);
  // printf("sizeof(Elf32_Shdr) = %ld sh_size = %d\n",sizeof(Elf32_Shdr),Elf_sec.sh_size);

  //get .symtab
  for(int n = 0; n < Elf_header.e_shnum; n ++){
    fseek(ftrace_fp,Elf_header.e_shoff + n * Elf_header.e_shentsize,SEEK_SET);
    ret = fread(&Elf_sec,Elf_header.e_shentsize,1,ftrace_fp);
    if (ret != 1) {
      perror("Error reading from file");
    }
    printf("Elf_sec.sh_name = %d\n",Elf_sec.sh_name);
    if(Elf_sec.sh_type == SHT_SYMTAB){
      printf("Elf_sec.sh_name = %d\n",Elf_sec.sh_name);
      sym_off = Elf_sec.sh_offset;
      continue;
    }
  }
  
  //读取.symtab
  fseek(ftrace_fp,sym_off,SEEK_SET);
  ret = fread(&Elf_sym,sizeof(Elf32_Sym),1,ftrace_fp);
  if (ret != 1) {
    printf("ret = %d\n",ret);
    perror("Error reading from file");
  }

  printf(".strtab : _%c_  length = %ld\n",strtab[1],strlen(strtab));
  printf("str_off = %d \n",str_off);
  printf("sym_off = %d\n",sym_off);
  printf("str_size = %ld\n",str_size);
  
  return 0;
}

//解析命令行参数
static int parse_args(int argc, char *argv[]) {
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
      case 'p': sscanf(optarg, "%d", &difftest_port); break;
      case 'l': log_file = optarg; break;
      case 'd': diff_so_file = optarg; break;
      case 'f': ftrace_file = optarg; break;
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

void init_monitor(int argc, char *argv[]) {
  /* Perform some global initialization. */

  /* Parse arguments. */
  parse_args(argc, argv);

  /* Set random time seed. */
  init_rand();

  /* Open the log file. */
  init_log(log_file);

  /* Initialize memory. */
  init_mem();

  /* Initialize devices. */
  IFDEF(CONFIG_DEVICE, init_device());

  /* Perform ISA dependent initialization. */
  init_isa();

  /* Load the image to memory. This will overwrite the built-in image. */
  long img_size = load_img();

  /* Initialize differential testing. */
  init_difftest(diff_so_file, img_size, difftest_port);

  /* Initialize the simple debugger.初始化简单调试器 */
  init_sdb();

  init_ftrace();

#ifndef CONFIG_ISA_loongarch32r
  IFDEF(CONFIG_ITRACE, init_disasm(
    MUXDEF(CONFIG_ISA_x86,     "i686",
    MUXDEF(CONFIG_ISA_mips32,  "mipsel",
    MUXDEF(CONFIG_ISA_riscv,
      MUXDEF(CONFIG_RV64,      "riscv64",
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif
  //init_ftrace();

  /* Display welcome message. */
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
