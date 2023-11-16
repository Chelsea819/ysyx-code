/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/

#include "svdpi.h"
#include "Vysyx_22041211_top__Dpi.h"
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vysyx_22041211_top.h"
#include "Vysyx_22041211_top___024root.h"

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

void set_npc_state(int state, vaddr_t pc, int halt_ret);
void invalid_inst(vaddr_t thispc);

NPCState npc_state = { .state = NPC_STOP };

static char *img_file = NULL;


#define MAX_SIM_TIME 100
#define PG_ALIGN __attribute((aligned(4096)))

#define BITMASK(bits) ((1ull << (bits)) - 1)
//位抽取
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
//符号扩展
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = x }; (uint64_t)__x.n; })

#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000
#define CONFIG_PC_RESET_OFFSET 0x0
#define CONFIG_RT_CHECK 1

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

vluint64_t sim_time = 0;
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
bool ifbreak = false;

uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

TOP_NAME dut;
VerilatedVcdC *m_trace = new VerilatedVcdC;

uint32_t convert_16(char *args);
uint32_t convert_ten(char *args);
static void watchPoints_display();

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  int times;
  uint32_t data;
  char *target;
  struct watchpoint *next;
  struct watchpoint *past;

  /* TODO: Add more members if necessary */

} WP;
static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;


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
  0x00000297,  // auipc t0,0
  0x00000297,  // auipc t0,0
  0x00100093,  // addi    x[1] = 0 + 1
  0x00000297,  // auipc t0,0
  0x00100073,  // ebreak (used as npc_trap)
  0x00100093,  // addi    x[1] = 0 + 1
};

void init_rand() {
  srand(time(0));
}

/* ------------------------------------reg.c------------------------------------ */

FILE *log_fp = NULL;

void init_log(const char *log_file) {
  log_fp = stdout;
  if (log_file != NULL) {
    FILE *fp = fopen(log_file, "w");
    Assert(fp, "Can not open '%s'", log_file);
    //Assert宏检查文件打开是否成功,如果失败则打印错误信息并退出
    
    log_fp = fp;
    //如果文件打开成功,则将log_fp指向这个文件
  }
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

uint8_t* guest_to_host_npc(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }

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


static vaddr_t load_mem_npc(paddr_t addr,int len) {
	if (likely(in_pmem(addr))) return pmem_read_npc(addr,len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void ifebreak_func(int inst){
	// printf("while key = 0x%08x\n",inst);
	if(inst == 1048691) {ifbreak = true; printf("ebreak-called: pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);} 
}

void mem_write_npc(vaddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) pmem_write_npc(addr, len, data);
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}


static void welcome() {
  printf("Welcome to NPC!\n");
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
void init_wp_pool();

void init_sdb()
{
  /* Compile the regular expressions. */
  // 编译正则表达式
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
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
      case 'b': sdb_set_batch_mode(); break; //sdb_set_batch_mode(); break;
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

    /* Open the log file. */
    init_log(log_file);

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

void set_npc_state(int state, vaddr_t pc, int halt_ret) {
//   difftest_skip_ref();
  npc_state.state = state;
  npc_state.halt_pc = pc;
  npc_state.halt_ret = halt_ret;
}



__attribute__((noinline))
void invalid_inst(vaddr_t thispc) {
  uint32_t temp[2];
  vaddr_t pc = thispc;
  temp[0] = load_mem_npc(pc, 4);
  temp[1] = load_mem_npc(pc, 4);

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

/* ------------------------------------cpu-exe.c------------------------------------ */

// static void trace_and_difftest(Decode *_this, vaddr_t dnpc)
// {
// #ifdef CONFIG_ITRACE_COND
//   if (ITRACE_COND)
//   {
//     log_write("%s\n", _this->logbuf);
//   }
// #endif
//   if (g_print_step)
//   {
//     IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
//   }
//   IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
//   // #ifdef CONFIG_WATCHPOINT
//   bool success = true;
//   uint32_t addr = 0;

//   WP *index = get_head();
//   while (index != NULL)
//   {
//     addr = expr(index->target, &success);
//     Assert(success,"Make_token fail!");
//     if(addr != index->data){
//       npc_state.state = NPC_STOP;
//       index->times += 1;
//       printf("\n\033[105m Hardware watchpoint %d: %s \033[0m\n", index->NO, index->target);
//       printf("Old value = %d\n", index->data);
//       printf("New value = %d\n\n", addr);
//       index->data = addr;
//       return;
//     }
//     index = index->next;
//     return;

//   }
//   // #endif
// }
word_t expr(char *e, bool *success);

static void trace_and_difftest(vaddr_t dnpc)
{
// #ifdef CONFIG_ITRACE_COND
//   if (ITRACE_COND)
//   {
//     log_write("%s\n", _this->logbuf);
//   }
// #endif
//   if (g_print_step)
//   {
//     IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
//   }
//   IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
  bool success = true;
  uint32_t addr = 0;

  WP *index = head;
  while (index != NULL)
  {
    addr = expr(index->target, &success);
    Assert(success,"Make_token fail!");

    if(addr != index->data){
      // printf("Different!\n");
      npc_state.state = NPC_STOP;
      index->times += 1;

      printf("\n\033[105m Hardware watchpoint %d: %s \033[0m\n", index->NO, index->target);
      printf("Old value = 0x%08x\n", index->data);
      printf("New value = 0x%08x\n\n", addr);

      index->data = addr;
      return;
    }

    index = index->next;
    }
    return;
}

char *convertTo_2(char args){
  char *result = (char *)malloc(5);
  int num = 0;

  if(args >= 'a' && args <= 'f'){
    num = (int)args - (int)'a' + 10;
  }
  else {
    num = (int)args - (int)'0';
  }

  int flag = 8;
  //15 -> 8 + 4 + 2 + 1
  //5 -> 4 + 1
  //0
  for(int n = 0; n < 4; n ++){
    //为0 则该位之后的低位均为0
    if(!num){
      for(; n < 4; n ++) result[n] = '0';
      break;
    }

    result[n] = num / flag + '0';
    if(num / flag) num -= flag;
    flag /= 2;
  }
  result[4] = '\0';
  return result;
}

/* let CPU conduct current command and renew PC */
static void exec_once(vaddr_t pc)
{
  //上升沿取指令
  if(dut.clk == 1) {
    if(dut.memWrite) mem_write_npc(dut.ALUResult,dut.DataLen + 1,dut.storeData);
    dut.inst = load_mem_npc(dut.pc,4);
    dut.eval();
    printf("common:pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);
    
  }
  if(dut.memToReg == 1){
			dut.ReadData = load_mem_npc(dut.ALUResult,dut.DataLen + 1);
			dut.eval();
		}
	m_trace->dump(sim_time);
	sim_time++;
		
  if(dut.invalid == 1){
    invalid_inst(dut.pc);
  }
  if(ifbreak && dut.clk == 0){
    printf("\nebreak!\n");
    printf("ebreak: pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);
    NPCTRAP(dut.pc, 0);
  }
}

/* stimulate the way CPU works ,get commands constantly */
static void execute(uint64_t n)
{
  // Decode s;
  for (; n > 0; n--)
  {
    exec_once(dut.pc);
    if(dut.clk == 1) g_nr_guest_inst++;  //记录客户指令的计时器
    trace_and_difftest(dut.pc);
    //当npc_state.state被设置为NPC_STOP时，npc停止执行指令
    if (npc_state.state != NPC_RUNNING)
      break;

    dut.clk ^= 1;
    dut.eval();
    // IFDEF(CONFIG_DEVICE, device_update());
  }
}

static void statistic()
{
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0)
    Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else
    Log("Finish running in less than 1 us and can not calculate the simulation frequency");
}

// void iringbuf_display(){
//   for(int i = 0;i <12 ; i++){
//     Assert(&(irbuf[i]) != NULL,"irbuf exists NULL!");
//     if(irbuf[i].rbuf == NULL) break;
//     if(irbuf + i == curre->past){
//       printf(" --->\t%s\n",irbuf[i].rbuf);
//       continue;
//     }
//     printf("\t%s\n",irbuf[i].rbuf);
//   }
// }

// void assert_fail_msg()
// {
//   iringbuf_display();
//   isa_reg_display();
//   statistic();
// }



/* Simulate how the CPU works. */
void cpu_exec(uint64_t n)
{
  // g_print_step = (n < MAX_INST_TO_PRINT);

  switch (npc_state.state){
			case NPC_END:
			case NPC_ABORT:
				printf("Program execution has ended. To restart the program, exit NPC and run again.\n");
				dut.final();
        m_trace->close();	//关闭波形跟踪文件
        return;
			default:
			npc_state.state = NPC_RUNNING;
		}	

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (npc_state.state){
			case NPC_RUNNING:
				npc_state.state = NPC_STOP;
				break;

			case NPC_END:
			case NPC_ABORT:
				Log("npc: %s at pc = " FMT_WORD,
					(npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
					npc_state.halt_pc);
			case NPC_QUIT:
				Log("quit!\n");
        statistic();
		}
}

/* ------------------------------------reg.c------------------------------------ */
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  for(int i = 0; i < 32; i++){
    printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,regs[i],gpr(i));
  }
  printf("\033[102m PC: \033[0m \t0x%08x\n",dut.pc);
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
    if(strcmp(regs[i],s) == 0){
      *success = true;
      free(s);
      return gpr(i);
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
      return load_mem_npc(convert_16(tokens[p + 1].str),4);
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

/* ------------------------------------watchpoint.c------------------------------------ */
//static int gap = 31;

WP* new_wp(char *args){
  //search if existed the same
  bool success = true;
  WP* p_searchExist = head;
  while(p_searchExist != NULL){
    if(strcmp(p_searchExist->target,args) == 0) {
      return NULL;
    }
    p_searchExist = p_searchExist->next;
  }

  //full
  if(free_==NULL)  Assert(0,"wp_pool full!\n");

  //find ava point
  WP* get_wp = free_;
  free_ = free_->next;
  // while(get_wp != NULL){
  //   if(get_wp->next == NULL){
  //     break;
  //   }
  //   get_wp = get_wp->next;
  // }

  //cut it from free_
  // get_wp->past->next = NULL;
  get_wp->times = 0;
  get_wp->target = (char *)malloc(strlen(args)+1);
  strcpy(get_wp->target,args);
  get_wp->data = expr(args,&success);
  Assert(success,"Make_token fail!");

  //add it to head list
  if(head == NULL){
    head = get_wp;
    get_wp->NO = 0;
    get_wp->next = NULL;
    get_wp->past = NULL;
    printf("the passing head address:%p\n",head);
  }
  else{
    WP* addSpot = head;
    while(addSpot->next != NULL){
      addSpot = addSpot->next;
    }
    addSpot->next = get_wp;
    get_wp->past = addSpot;
    get_wp->NO = addSpot->NO + 1;
    get_wp->next = NULL;
  }

    WP *index = head;
    while(index){
      printf("\n head \nNO:%d  target:%s   %p   data: %x\n",index->NO,index->target,index->target,index->data);
      index = index->next;
    }
    //printf("head : %p\n",head);
  //printf("head : %p\n",head->next);
  return get_wp;
}

void free_wp(WP *wp){
  if(!wp) Assert(0,"Free_wp received NULL!\n");

  //remove it from head
  if(wp->next == NULL && wp->past == NULL) head = NULL;
  else{
    if(wp->past != NULL) wp->past->next = wp->next;
    else head = wp->next;
    if(wp->next != NULL) wp->next->past = wp->past;
    //printf("Remove it from %d in head\n",wp->NO);
  }

  free(wp->target);

  wp->data = 0;
  wp->times = 0;

  //add it to free_
  if(!free_) {
    free_ = wp;
    wp->next = NULL;
    wp->NO = 0;
    wp->past = NULL;
    return;
  }
  
  WP *index = free_;
  while(index != NULL){
    if(index->next == NULL){
      index->next = wp;
      wp->past = index;
      wp->next = NULL;
      wp->NO = index->NO + 1;
      break;
    }
    index = index->next;
  }
  //printf("ADD it to %d in free_",wp->NO);
  return;


}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].past = (i == 0 ? NULL : &wp_pool[i - 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */




/* ------------------------------------sdb.c------------------------------------ */

static int is_batch_mode = false;

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(npc) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static char too_lessArg(char *args){
  if(args == NULL) {
    Log("Without necessary arguments!");
    return 1;
  }
  return 0;
}

static int cmd_c(char *args)
{
  /*Simulate how the CPU works.*/
  cpu_exec(-1);
  return 0;
}

uint32_t convert_ten(char *args){
  uint32_t flag = 1;
  uint32_t flag_neg = 0;
  uint32_t n = 0;

  int i = strlen(args) - 1;
  for(  ;i >= 0;i --){
    if(args[i] == 'n'){
      flag_neg += 1;
      continue;
    }
    n += ((int)args[i] - (int)'0') * flag;
    flag = flag * 10;
  }
  while(flag_neg --) n = n * (-1);
  return n;
}
//0x80008ffc

uint32_t convert_16(char *args){
  uint32_t addr = 0;
  uint32_t flag = 1;
  for(int i = strlen(args) - 1;i >= 2;i --){
    if(args[i] >= 'a' && args[i] <= 'f'){
      addr += ((int)args[i] - (int)'a' + 10) * flag;
    }
    else {
      addr += ((int)args[i] - (int)'0') * flag;
  }
    flag *= 16;
  }
  return addr;
}

static int cmd_si(char *args){
  uint64_t n = 0;
  if(args == NULL)  n = 1;
  else {
    for(int i = 0; i < strlen(args) ; i ++){
      if(args[i] > '9' || args[i] < '0'){
        Log("Please input number!");
        return 0;
      }
    }
    n = convert_ten(args);
  }
  cpu_exec(n*2);
  return 0;
}

static int cmd_q(char *args)
{
  npc_state.state = NPC_QUIT;
  return -1;
}

static void watchPoints_display(){
  WP *index = head;
  if(index == NULL ) {
    printf("Now, no WP in watchPool!\n");
    return;
  }
  //printf("head : %p\n",index);
  //printf("head->next : %p\n",index->next);
  printf("\033[92m Num \tTYpe \tDisp \tEnb \tAddress \t What \033[m \n");
  while(index != NULL){
    printf("\033[92m %d \thw watchpoint \tkeep \ty \t %s \033[m \n",index->NO,index->target);
    printf("\033[96m \tbreakpoint already hit %d time \033[m \n",index->times);
    index = index->next;
  }
  return;
}

static int cmd_w(char *args){
  if(too_lessArg(args) == 1) return 0;
  WP* ret =new_wp(args);
  if(ret==NULL) printf("%s already in wp_pool!\n",args); 
  watchPoints_display();
  return 0;
}

static int cmd_pcount(char *args){
  bool success = true;
  if(too_lessArg(args) == 1) {
    return 0;
  }
  word_t result = expr(args,&success);
  printf("\033[105m [ %s ] = [%u] [0x%08x]\033[0m\n",args,result,result);
  Assert(success,"Make_token fail!");
  return 0;
}


static int cmd_x(char *args){
  if(too_lessArg(args) == 1) return 0;

  char *arg1 = strtok(NULL," ");
  char *arg2 = strtok(NULL," ");
  if(too_lessArg(arg2) == 1) return 0;

  int len = convert_ten(arg1);
  vaddr_t addr = convert_16(arg2);
  printf("addr = %08x\n",addr);

  for (int i = 0;i < len;i ++){
    printf("\033[105m 0x%08x: \033[0m \t0x%08x\n",addr + i,load_mem_npc(addr + i,4));
  }
  return 0; 
}



static int cmd_info(char *args){
  if (too_lessArg(args) == 1) return 0; 
  else if (*args == 'r')  isa_reg_display();
  else if (*args == 'w')  watchPoints_display();
  else Log("Unknown command '%s'", args);
  return 0;
}


static int cmd_help(char *args);

static int cmd_d(char *args){
  if(too_lessArg(args) == 1) return 0;
  WP *index = head;
  while(index != NULL){
    if(convert_ten(args) == index->NO){
      free_wp(index);
      watchPoints_display();

      return 0;;
    }
    index = index->next;
  }
  printf("No %s in watchpool!\n",args);
  watchPoints_display();
  return 0;
}


static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NPC", cmd_q},
    {"si", "Execuate one by one.",cmd_si},
    {"info","Print the state of register(r) or the content of watchpoint(w)",cmd_info},
    {"p","Get the result of EXPR.",cmd_pcount},
    {"x","Scan the memory.",cmd_x},
    {"w","Set a watchpoint.",cmd_w},
    {"d","Delete certain watchpoint.",cmd_d},

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

/* Receive commands from user. */
void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    //return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
  WP *head = head;
  while(head != NULL){
    free(head->target);
    head->target = NULL;
    head = head->next;
  }
  // iringbuf *head_i = get_head_iringbuf();
  // while(head_i != NULL && head_i->rbuf != NULL){
  //   free(head_i->rbuf);
  //   head_i->rbuf = NULL;
  //   head_i = head_i->next;
  // }
  
}

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
	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
	

	init_npc(argc, argv);

	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
	
	dut.clk = 0; 
	dut.eval();
	dut.rst = 1;
	dut.eval();

  m_trace->dump(sim_time);
	sim_time++;

  dut.clk = 1;
  dut.eval();
  dut.rst = 0;
	dut.eval();

  /* Start engine. */
	engine_start();

  // std::cout << dut.rootp->ysyx_22041211_top__DOT__my_RegisterFile__DOT__rf[1] << std::endl;
  printf("register:0x%08x",dut.rootp->ysyx_22041211_top__DOT__my_RegisterFile__DOT__rf[1]);

	// while(1){
	// 	switch (npc_state.state){
	// 		case NPC_END:
	// 		case NPC_ABORT:
	// 			printf("Program execution has ended. To restart the program, exit NPC and run again.\n");
	// 			dut.final();
  //       m_trace->close();	//关闭波形跟踪文件
  //       exit(EXIT_SUCCESS);
	// 		default:
	// 		npc_state.state = NPC_RUNNING;
	// 	}		
		
	// 	//上升沿取指令
	// 	if(dut.clk == 1) {
	// 		if(dut.memWrite) mem_write_npc(dut.ALUResult,dut.DataLen + 1,dut.storeData);
	// 		dut.inst = load_mem_npc(dut.pc,4);
	// 		dut.eval();
  //     printf("common:pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);
			
	// 	}
		
	// 	if(dut.memToReg == 1){
	// 		dut.ReadData = load_mem_npc(dut.ALUResult,dut.DataLen + 1);
	// 		dut.eval();
	// 	}
	// 	m_trace->dump(sim_time);
	// 	sim_time++;
		
	// 	if(dut.invalid == 1){
	// 		invalid_inst(dut.pc);
	// 	}
	// 	if(ifbreak && dut.clk == 0){
	// 		printf("\nebreak!\n");
  //     printf("ebreak: pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);
	// 		NPCTRAP(dut.pc, 0);
	// 	}

	// 	switch (npc_state.state){
	// 		case NPC_RUNNING:
	// 			npc_state.state = NPC_STOP;
	// 			break;

	// 		case NPC_END:
	// 		case NPC_ABORT:
	// 			Log("npc: %s at pc = " FMT_WORD,
	// 				(npc_state.state == NPC_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : (npc_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
	// 				npc_state.halt_pc);
	// 		case NPC_QUIT:
	// 			Log("quit!\n");
	// 	}
  //   printf("\n");
    
  //   dut.clk ^= 1;
	// 	dut.eval();

	// }

	dut.final();
	m_trace->close();	//关闭波形跟踪文件
	exit(EXIT_SUCCESS);
}



