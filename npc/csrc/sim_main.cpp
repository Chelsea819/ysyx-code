/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/

#include "sim.h"

#include<iostream>

#include <readline/readline.h>
#include <readline/history.h>
#include <locale.h>
#include <time.h>
#include <getopt.h>
#include <regex.h>
#include "utils.h"
#include "common.h"
#include "reg.h"
#include "cpu.h"
#include "decode.h"
#include <elf.h>
#include "difftest-def.h"
#include "config.h"
#include "device-def.h"
#include "device/map.h"
#include "device/mmio.h"
#include "sdb.h"


void init_device();
void set_npc_state(int state, vaddr_t pc, int halt_ret);
void invalid_inst(vaddr_t thispc);
void init_disasm(const char *triple);
#ifdef CONFIG_DIFFTEST
void init_difftest(char *ref_so_file, long img_size, int port);
#endif
void difftest_step(vaddr_t pc, vaddr_t npc);
void init_sdb();
void sdb_mainloop();
NPCState npc_state = { .state = NPC_STOP };

static char *img_file = NULL;
static char *ftrace_file = NULL;
static int difftest_port = 1234;
static char *diff_so_file = NULL;

#define MAX_SIM_TIME 100
#define PG_ALIGN __attribute((aligned(4096)))

#define BITMASK(bits) ((1ull << (bits)) - 1)
//位抽取
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
//符号扩展
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = x }; (uint64_t)__x.n; })

#define CHOOSE2nd(a, b, ...) b
#define MUX_WITH_COMMA(contain_comma, a, b) CHOOSE2nd(contain_comma a, b)
#define MUX_MACRO_PROPERTY(p, macro, a, b) MUX_WITH_COMMA(concat(p, macro), a, b)
#define MUXDEF(macro, X, Y)  MUX_MACRO_PROPERTY(__P_DEF_, macro, X, Y)

// simplification for conditional compilation
#define __IGNORE(...)
#define __KEEP(...) __VA_ARGS__
// keep the code if a boolean macro is defined
#define IFDEF(macro, ...) MUXDEF(macro, __KEEP, __IGNORE)(__VA_ARGS__)

typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef word_t vaddr_t;

#define PMEM_LEFT  ((paddr_t)CONFIG_MBASE)
#define PMEM_RIGHT ((paddr_t)CONFIG_MBASE + CONFIG_MSIZE - 1)
#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)

#define NPCTRAP(thispc, code) set_npc_state(NPC_END, thispc, code)

// #define __GUEST_ISA__ riscv32

vluint64_t sim_time = 0;
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

TOP_NAME dut;
extern WP *head;

#ifdef CONFIG_WAVE
VerilatedVcdC *m_trace = new VerilatedVcdC;
#endif

uint32_t convert_16(char *args);
uint32_t convert_ten(char *args);
static void watchPoints_display();
int init_ftrace(const char *ftrace_file);

/* ------------------------------------timer------------------------------------ */

#include MUXDEF(CONFIG_TIMER_GETTIMEOFDAY, <sys/time.h>, <time.h>)

IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(CLOCKS_PER_SEC == 1000000, "CLOCKS_PER_SEC != 1000000"));
IFDEF(CONFIG_TIMER_CLOCK_GETTIME,
    static_assert(sizeof(clock_t) == 8, "sizeof(clock_t) != 8"));

static uint64_t boot_time = 0;

static uint64_t get_time_internal() {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC_COARSE, &now);
  uint64_t us = now.tv_sec * 1000000 + now.tv_nsec / 1000;
  return us;
}

uint64_t get_time() {
  if (boot_time == 0) boot_time = get_time_internal();
  uint64_t now = get_time_internal();
  return now - boot_time;
}

/*
       _                         __  __                         _ 
      (_)                       |  \/  |                       | |
  _ __ _ ___  ___ ________   __ | \  / | __ _ _ __  _   _  __ _| |
 | '__| / __|/ __|______\ \ / / | |\/| |/ _` | '_ \| | | |/ _` | |
 | |  | \__ \ (__        \ V /  | |  | | (_| | | | | |_| | (_| | |
 |_|  |_|___/\___|        \_/   |_|  |_|\__,_|_| |_|\__,_|\__,_|_|

*/
unsigned char isa_logo[] = {
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x5f, 0x5f, 0x20,
  0x20, 0x5f, 0x5f, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x5f, 0x20, 0x0a, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x28, 0x5f, 0x29, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x7c, 0x20, 0x20, 0x5c, 0x2f, 0x20, 0x20, 0x7c, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x7c, 0x20,
  0x7c, 0x0a, 0x20, 0x20, 0x5f, 0x20, 0x5f, 0x5f, 0x20, 0x5f, 0x20, 0x5f,
  0x5f, 0x5f, 0x20, 0x20, 0x5f, 0x5f, 0x5f, 0x20, 0x5f, 0x5f, 0x5f, 0x5f,
  0x5f, 0x5f, 0x5f, 0x5f, 0x20, 0x20, 0x20, 0x5f, 0x5f, 0x20, 0x7c, 0x20,
  0x5c, 0x20, 0x20, 0x2f, 0x20, 0x7c, 0x20, 0x5f, 0x5f, 0x20, 0x5f, 0x20,
  0x5f, 0x20, 0x5f, 0x5f, 0x20, 0x20, 0x5f, 0x20, 0x20, 0x20, 0x5f, 0x20,
  0x20, 0x5f, 0x5f, 0x20, 0x5f, 0x7c, 0x20, 0x7c, 0x0a, 0x20, 0x7c, 0x20,
  0x27, 0x5f, 0x5f, 0x7c, 0x20, 0x2f, 0x20, 0x5f, 0x5f, 0x7c, 0x2f, 0x20,
  0x5f, 0x5f, 0x7c, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5f, 0x5c, 0x20, 0x5c,
  0x20, 0x2f, 0x20, 0x2f, 0x20, 0x7c, 0x20, 0x7c, 0x5c, 0x2f, 0x7c, 0x20,
  0x7c, 0x2f, 0x20, 0x5f, 0x60, 0x20, 0x7c, 0x20, 0x27, 0x5f, 0x20, 0x5c,
  0x7c, 0x20, 0x7c, 0x20, 0x7c, 0x20, 0x7c, 0x2f, 0x20, 0x5f, 0x60, 0x20,
  0x7c, 0x20, 0x7c, 0x0a, 0x20, 0x7c, 0x20, 0x7c, 0x20, 0x20, 0x7c, 0x20,
  0x5c, 0x5f, 0x5f, 0x20, 0x5c, 0x20, 0x28, 0x5f, 0x5f, 0x20, 0x20, 0x20,
  0x20, 0x20, 0x20, 0x20, 0x20, 0x5c, 0x20, 0x56, 0x20, 0x2f, 0x20, 0x20,
  0x7c, 0x20, 0x7c, 0x20, 0x20, 0x7c, 0x20, 0x7c, 0x20, 0x28, 0x5f, 0x7c,
  0x20, 0x7c, 0x20, 0x7c, 0x20, 0x7c, 0x20, 0x7c, 0x20, 0x7c, 0x5f, 0x7c,
  0x20, 0x7c, 0x20, 0x28, 0x5f, 0x7c, 0x20, 0x7c, 0x20, 0x7c, 0x0a, 0x20,
  0x7c, 0x5f, 0x7c, 0x20, 0x20, 0x7c, 0x5f, 0x7c, 0x5f, 0x5f, 0x5f, 0x2f,
  0x5c, 0x5f, 0x5f, 0x5f, 0x7c, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
  0x20, 0x5c, 0x5f, 0x2f, 0x20, 0x20, 0x20, 0x7c, 0x5f, 0x7c, 0x20, 0x20,
  0x7c, 0x5f, 0x7c, 0x5c, 0x5f, 0x5f, 0x2c, 0x5f, 0x7c, 0x5f, 0x7c, 0x20,
  0x7c, 0x5f, 0x7c, 0x5c, 0x5f, 0x5f, 0x2c, 0x5f, 0x7c, 0x5c, 0x5f, 0x5f,
  0x2c, 0x5f, 0x7c, 0x5f, 0x7c, 0x0a, '\0'  /* Termination Character is indispensable! */
};

static const uint32_t img [] = {
  0x00010537,  // lui	a0,0x10
  0x01050513,  // auipc t0,0
  0x00010537,  // addi    x[1] = 0 + 1
  0x01050513,  // auipc t0,0
  0x00100073,  // ebreak (used ass npc_trap)
  0x00100093,  // addi    x[1] = 0 + 1
};

void init_rand() {
  srand(time(0));
}

/* ------------------------------------log.c------------------------------------ */
FILE *log_fp = NULL;
static char *log_file = NULL;

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

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, dut.pc);
}

void init_mem_npc(){
    uint32_t *p = (uint32_t *)pmem;
    int i;
    for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
        p[i] = rand();
    }
	return ;
}

uint8_t* guest_to_host_npc(paddr_t paddr) { 
  // printf("pmem: 0x%08x\n",pmem);
  // printf("paddr: 0x%08x\n",paddr);
  // printf("CONFIG_MBASE: 0x%08x\n",CONFIG_MBASE);
  return pmem + paddr - CONFIG_MBASE; 
}

void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host_npc(RESET_VECTOR), img, sizeof(img));
}

static inline word_t host_read_npc(void *addr, int len) {
  switch (len) {
    case 1: return *(uint8_t  *)addr;
    case 2: return *(uint16_t *)addr;
    case 4: return *(uint32_t *)addr;
    IFDEF(CONFIG_ISA64, case 8: return *(uint64_t *)addr);
    default: MUXDEF(CONFIG_RT_CHECK, assert(0), return 0);
  }
}

static inline void host_write_npc(void *addr, int len, word_t data) {
  switch (len) {
    case 1: *(uint8_t  *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
    IFDEF(CONFIG_ISA64, case 8: *(uint64_t *)addr = data; return);
    IFDEF(CONFIG_RT_CHECK, default: assert(0));
  }
}

static inline bool in_pmem(paddr_t addr) {
  return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

static word_t pmem_read_npc(paddr_t addr,int len) {
  word_t ret = host_read_npc(guest_to_host_npc(addr), len);
  return ret;
}

static void pmem_write_npc(paddr_t addr, int len, word_t data) {
  host_write_npc(guest_to_host_npc(addr), len, data);
}


vaddr_t paddr_read(paddr_t addr,int len) {
  
	if (likely(in_pmem(addr))) {
    word_t rdata = pmem_read_npc(addr,len);
    #ifdef CONFIG_MTRACE
      Log("paddr_read ---  [addr: 0x%08x len: %d rdata: 0x%08x]",addr,len,rdata);
    #endif
    return rdata;
  }
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(vaddr_t addr, vaddr_t len, word_t data) {
  #ifdef CONFIG_MTRACE
  Log("paddr_write --- [addr: 0x%08x len: %d data: 0x%08x]",addr,len,data);
  #endif
  if (likely(in_pmem(addr))) { return pmem_write_npc(addr, len, data);}
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}

extern "C" int pmem_read(int raddr, char wmask) {
  // 总是读取地址为`raddr & ~0x3u`的4字节返回给`rdata`
  // printf("read!\n");
  // printf("raddr = 0x%08x\n",raddr); 
  // vaddr_t rdata = paddr_read((paddr_t)(raddr & ~0x3u), 4);
  // printf("rdata = 0x%08x\n",rdata);
  int len = 0;
  switch (wmask){
      case 0x1: len = 1; break;
      case 0x3: len = 2; break;
      case 0xf: len = 4; break;
      IFDEF(CONFIG_ISA64, case 0x8: len = 8; return);
      IFDEF(CONFIG_RT_CHECK, default: assert(0));
    }
  if(raddr == CONFIG_RTC_MMIO || raddr == CONFIG_SERIAL_MMIO) { 
    // Log("Read device --- [addr: 0x%08x  len: %d]",raddr,len);  
    // time_t current_time;
    // time(&current_time); // 获取系统时间戳
    // return current_time;
  }
  return paddr_read((paddr_t)raddr, len);
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  // 总是往地址为`waddr & ~0x3u`的4字节按写掩码`wmask`写入`wdata`
  // `wmask`中每比特表示`wdata`中1个字节的掩码,
  // 如`wmask = 0x3`代表只写入最低2个字节, 内存中的其它字节保持不变
  // printf("pc = 0x%08x\n",dut.pc);
  // printf("wmask = 0x%01u\n",wmask);
  // printf("waddr = 0x%08x\n",(paddr_t)waddr);
  // printf("wdata = 0x%08x\n",(paddr_t)wdata);
  if(waddr == CONFIG_SERIAL_MMIO) {
    // Log("Write device --- [addr: 0x%08x data: 0x%08x]",waddr,wdata);
    // putchar(wdata);
  }
  // else {
    int len = 0;
    switch (wmask){
      case 0x1: len = 1; break;
      case 0x3: len = 2; break;
      case 0xf: len = 4; break;
      IFDEF(CONFIG_ISA64, case 0x8: len = 8; return);
      IFDEF(CONFIG_RT_CHECK, default: assert(0));
    }
    paddr_write((vaddr_t)waddr, (vaddr_t)len, (word_t)wdata);
  // }
}

extern bool ifbreak;
void ifebreak_func(int inst){
	// printf("while key = 0x%08x\n",inst);printf("ebreak-called: pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst)
	if(inst == 1048691) {ifbreak = true; } 
}


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
  int ret = fread(guest_to_host_npc(RESET_VECTOR), size, 1, fp);
//   printf("inst = 0x%08x\n",*(uint32_t*)(guest_to_host_npc(RESET_VECTOR)));
  //fread()可以高效地从文件流中读取大块的二进制数据,放入指定的内存缓冲区中
  assert(ret == 1);

  fclose(fp);
  return size;
}

enum {
  TK_NOTYPE = 256,  //space
  TK_LEFT_BRA, TK_RIGHT_BRA, //bracket
  TK_NUM, TK_HEXA, TK_REG,DEREF, //data
  //3优先级 * /
  TK_MUL,TK_DIV, 
  //4优先级 + - 
  TK_ADD,TK_SUB,
  //7优先级 == !=
  TK_EQ, TK_NEQ,
  //11优先级&&
  TK_AND,

  /* TODO: Add more token types */

};

//实现了一个基于正则表达式匹配的词法解析规则定义
static struct rule {

  //用于匹配的正则表达式
  const char *regex;
  //匹配成功后对应的语法标记类型
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"\\+", TK_ADD},      // plus
  {"-",TK_SUB},         // sub
  {"\\*",TK_MUL},       // mul
  {"/",TK_DIV},         // division
  {"\\(",TK_LEFT_BRA},    // bracket-left
  {"\\)",TK_RIGHT_BRA},   // bracket-right
  {"^0x[a-z0-9]+",TK_HEXA},
  {"[0-9]+",TK_NUM},    // num
  {"^\\$+[a-z0-9]+",TK_REG},
  {"!=",TK_NEQ},
  {"&&",TK_AND},
};

#define NR_REGEX ARRLEN(rules)

//collect the inner content after compling regex
static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
//这样可以预先编译所有正则模式,提高后续匹配效率
//检查每个正则表达式是否编译成功,避免在匹配时才发现错误
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  //遍历正则规则数组rules[],共有NR_REGEX条规则
  for (i = 0; i < NR_REGEX; i ++) {
    /*第三个参数是匹配选项,这里是REG_EXTENDED表示支持扩展正则语法*/
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      printf("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
      assert(0);
    }
  }
}

void sdb_set_batch_mode();

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
    init_mem_npc();

    /* Initialize devices. */
    IFDEF(CONFIG_DEVICE, init_device());

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
                               "riscv32"),
                               "bad"))) "-pc-linux-gnu"
  ));
#endif
    welcome();
}

void set_npc_state(int state, vaddr_t pc, int halt_ret) {
  difftest_skip_ref();
  npc_state.state = state;
  npc_state.halt_pc = pc;
  npc_state.halt_ret = halt_ret;
}



__attribute__((noinline))
void invalid_inst(vaddr_t thispc) {
  uint32_t temp[2];
  vaddr_t pc = thispc;
  temp[0] = paddr_read(pc, 4);
  temp[1] = paddr_read(pc, 4);

  uint8_t *p = (uint8_t *)temp;
  printf("invalid opcode(PC = " FMT_WORD "):\n"
      "\t%02x %02x %02x %02x %02x %02x %02x %02x ...\n"
      "\t%08x %08x...\n",
      thispc, p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], temp[0], temp[1]);

  printf("There are two cases which will trigger this unexpected exception:\n"
      "1. The instruction at PC = " FMT_WORD " is not implemented.\n"
      "2. Something is implemented incorrectly.\n", thispc);
  printf("Find this PC(" FMT_WORD ") in the disassembling result to distinguish which case it is.\n\n", thispc);
  printf(ANSI_FMT("If it is the first case, see\n%s\nfor more details.\n\n"
        "If it is the second case, remember:\n"
        "* The machine is always right!\n"
        "* Every line of untested code is always wrong!\n\n", ANSI_FG_RED), isa_logo);
  set_npc_state(NPC_ABORT, thispc, -1);
}


/* ------------------------------------reg.c------------------------------------ */
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  if(cpu.pc == 0x80000000) return;
  for(int i = 0; i < 32; i++){
    printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,reg_name(i),R(i));
  }
  printf("\033[102m PC: \033[0m \t0x%08x\n",cpu.pc);
  return;
}

word_t isa_reg_str2val(char *s, bool *success) {
  //printf("args = _%s_\n",s);
  if(strcmp("pc",s) == 0){
      *success = true;
      //printf("strcmp(pc,s) == 0\n");
      free(s);
      return dut.pc;
  }  
  for(int i = 0; i < 32; i++){
    if(strcmp(reg_name(i),s) == 0){
      *success = true;
      free(s);
      return R(i);
    }
  } 
  return 0;
}
/* ------------------------------------expr.c------------------------------------ */

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  //current position
  memset(tokens, 0, sizeof(tokens));
  int position = 0;
  int i;
  //Byte offset from string's start to substring' starting and end
  regmatch_t pmatch;

  nr_token = 0;
  int flag_neg = 0;
  while (e[position] != '\0') {
    
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        //Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            //i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        //TK_NOTYPE = 256, TK_EQ, TK_SUB,TK_MUL,
        //TK_DIV, TK_LEFT_BRA, TK_RIGHT_BRA, TK_NUM

        switch (rules[i].token_type) {
          case TK_NOTYPE:
            break;

          case TK_SUB:
            if( (nr_token == 0) 
            || (nr_token > 0 && 
              ((tokens[nr_token - 1].type >= TK_MUL && tokens[nr_token - 1].type <= TK_AND) || tokens[nr_token - 1].type == TK_LEFT_BRA)))
            {
              Log("Get the negtive num! nr_token = %d",nr_token);
              flag_neg += 1;
              break;
            } 

          case TK_MUL:
          case TK_DIV:
          case TK_ADD:
          case TK_LEFT_BRA:
          case TK_RIGHT_BRA:
          case TK_AND:
          case TK_NEQ:
          case TK_EQ:
            tokens[nr_token ++].type = rules[i].token_type;
            break;

          //十进制数（进行了对正负的处理）
          case TK_NUM: 
            tokens[nr_token].type = rules[i].token_type;
            assert(substr_len <= 31);
            strncpy(tokens[nr_token].str, substr_start, substr_len);
            while(flag_neg --) strcat(tokens[nr_token].str,"n"); 
            nr_token++;
            flag_neg = 0;
            break;

          //十六进制数
          case TK_HEXA:
            tokens[nr_token].type = rules[i].token_type;
            assert(substr_len <= 32);
            strncpy(tokens[nr_token++].str, substr_start, substr_len);
            break;

          case TK_REG:
            tokens[nr_token].type = rules[i].token_type;
            assert(substr_len <= 32);
            strncpy(tokens[nr_token++].str, substr_start + 1, substr_len);
            break;


          default: TODO();
        }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

static int op_type = 262;

static int find_main(int p,int q){
  //printf("enter main_find p = %d, q = %d\n",p ,q);
  int flag = 0;
  int flag_bracket = 0;
  op_type = 262;
  for(int i = q ; i > p ; i -- ){
    if(tokens[i].type == TK_RIGHT_BRA){
      flag_bracket += 1;
      continue;
    }
    else if(tokens[i].type == TK_LEFT_BRA){
      flag_bracket -= 1;
      continue;
    }
    if(flag_bracket == 0){
      if(tokens[i].type == TK_AND) {
        op_type = tokens[i].type;
        return i;
      }
      if(tokens[i].type > op_type) {
        op_type = tokens[i].type;
        flag = i;
        //printf("current op_type = %d flag = %d\n",op_type,flag);
      }
    }
  }
  //printf("get main_find op_type = %d flag = %d\n",op_type,flag);
  return flag;
}

bool check_parentheses(int p, int q){
  //printf("enter check\n");
  int left = 0;
  int right = 0;
  int fake_false = 0;
  if(tokens[p].type != TK_LEFT_BRA  || tokens[q].type != TK_RIGHT_BRA)      return false;
  else {
    for(int i = p; i <= q ; i ++){
      // if(left == 1 && right == 0 && tokens[i].type == TK_RIGHT_BRA) {
      //   fake_false = 1;
      // }
      if(left == right && left!=0  && i != q ) {
        fake_false = 1;
        //printf("fake_false = 1\n");
      }
      if(tokens[i].type == TK_RIGHT_BRA) {
        right += 1;
        //printf("right = %d  i = %d \n",right,i);
        if(right > left)  {
          //printf("right = %d left = %d i = %d \n",right,left,i);
          Assert(0,"Bad expression--too much right brackets");}
        }
      else if(tokens[i].type == TK_LEFT_BRA) {
        left += 1;
        //printf("left = %d  i = %d \n",left,i);
      }

    }
    if(left != right ) Assert(0,"Bad expression--too much left brackets,left = %d right = %d\n",left,right);
  }
  if(fake_false) return false;
  return true;
}   



uint32_t eval(int p, int q){
  //int num = 0;
  uint32_t val1 = 0;
  uint32_t val2 = 0;
  int op_type1 = 0;
  int op = 0;
  bool success = false;
  char *arr1 = (char*)(malloc(3 * sizeof(char)));
  
  //int error = 0;
  //printf("initial p = %d ,q = %d\n",p,q);
  if (p > q) {
    /* Bad expression */
    Assert(0,"Bad expression!");
  }
  else if (p == q) {
    /* Single token.
     * For now this token should be a number.
     * Return the value of the number.
     */
    switch (tokens[p].type){

    case TK_NUM: return convert_ten(tokens[p].str);

    //寄存器 or pc 里的值
    case TK_REG: 
      //printf("pc current\n");
      memset(arr1,0,3*sizeof(char));
      strncpy(arr1,tokens[p].str,2);
      
      return isa_reg_str2val(arr1, &success);

    //十六进制数 
    case TK_HEXA: 
      return convert_16(tokens[p].str);
    
    default:
      Assert(0,"Unknown content!\n");
    }
    
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    //printf("check_parentheses(p, q) == true, p = %d , q = %d\n",p,q);
    return eval(p + 1, q - 1);
  }
  else {
    /* We should do more things here. */
    //printf("before main finding\n");
    if(tokens[p].type == DEREF){
      return paddr_read(convert_16(tokens[p + 1].str),4);
    }
    op = find_main(p,q);
    op_type1 = op_type;
    val1 = eval(p, op - 1);
    //printf("val1 = 0x%08x\n",val1);
    val2 = eval(op + 1, q);
    //printf("val2 = 0x%08x\n",val2);
    //printf("find_main op_type: %d\n",op_type);

    switch (op_type1) {
      case TK_ADD: return val1 + val2;
      case TK_SUB: return val1 - val2;
      case TK_MUL: return val1 * val2;
      case TK_DIV: return val1 / val2;
      case TK_AND: return val1 && val2; 
      case TK_NEQ: return val1 != val2;
      case TK_EQ:  return val1 == val2;
      default: assert(0);
    
    }
  }
  return 0;
}


word_t expr(char *e, bool *success) {
  int i = 0;
  if (!make_token(e)) {
    *success = false;
    return 0;
  }
  /* TODO: Insert codes to evaluate the expression. */
  //TODO(); 

  for (i = 0; i < 32; i ++) {
    if(tokens[i].type == 0) {
      break;
    }
    if (tokens[i].type == TK_MUL && 
    (i == 0 || 
    tokens[i - 1].type >=263 )
    ) {
      tokens[i].type = DEREF;
    }
  }
  return eval(0, i - 1);
}



/* ------------------------------------sdb.c------------------------------------ */



/* start CPU or receive commands */
void engine_start() {
#ifdef CONFIG_TARGET_AM
/* Simulate how the CPU works. */
  cpu_exec(-1);
#else
  /* Receive commands from user. */
  sdb_mainloop();
#endif
}

int main(int argc, char** argv, char** env) {
#ifdef CONFIG_WAVE
	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
#else	
	Verilated::traceEverOn(false); 
#endif
	init_npc(argc, argv);

#ifdef CONFIG_WAVE
	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
#endif	

	dut.clk = 0; 
	dut.eval();
	dut.rst = 1;
	dut.eval();
#ifdef CONFIG_WAVE
  m_trace->dump(sim_time);
	sim_time++;
#endif	
  dut.clk = 1;
  dut.eval();
  dut.rst = 0;
	dut.eval();
#ifdef CONFIG_WAVE
  m_trace->dump(sim_time);
	sim_time++;
#endif
  /* Start engine. */
	engine_start();

	dut.final();
#ifdef CONFIG_WAVE 
	m_trace->close();	//关闭波形跟踪文件
#endif
	exit(EXIT_SUCCESS);
}



