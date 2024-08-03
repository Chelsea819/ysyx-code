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


void init_device();
void set_npc_state(int state, vaddr_t pc, int halt_ret);
void invalid_inst(vaddr_t thispc);
void init_disasm(const char *triple);
#ifdef CONFIG_DIFFTEST
void init_difftest(char *ref_so_file, long img_size, int port);
#endif
void difftest_step(vaddr_t pc, vaddr_t npc);

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
bool ifbreak = false;

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

TOP_NAME dut;

#ifdef CONFIG_WAVE
VerilatedVcdC *m_trace = new VerilatedVcdC;
#endif

uint32_t convert_16(char *args);
uint32_t convert_ten(char *args);
static void watchPoints_display();
int init_ftrace(const char *ftrace_file);

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


static vaddr_t paddr_read(paddr_t addr,int len) {
  
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

/* ------------------------------------cpu-exe.c------------------------------------ */
word_t expr(char *e, bool *success);
Decode s;
Decode diff;
FILE *ftrace_fp = NULL;

Elf32_Ehdr Elf_header;
Elf32_Shdr Elf_sec;
Elf32_Off sym_off;
Elf32_Off str_off;
Elf32_Sym Elf_sym;
Elf32_Xword str_size;
Elf32_Xword sym_size;
int sym_num;
char *strtab = NULL;


#ifdef CONFIG_FTRACE
int init_ftrace(const char *ftrace_file)
{
  FILE *fp = NULL;

  // 检查文件是否能正常读取
  Assert(ftrace_file, "ftrace_file is NULL!\n");

  fp = fopen(ftrace_file, "r");

  printf("ftrace_file :%s\n",ftrace_file);

  Assert(fp, "Can not open '%s'", ftrace_file);

  ftrace_fp = fp;

  // 读取ELF header
  int ret = fread(&Elf_header, sizeof(Elf32_Ehdr), 1, ftrace_fp);
  if (ret != 1)
  {
    perror("Error reading from file");
  }
  if (Elf_header.e_ident[0] != '\x7f' || memcmp(&(Elf_header.e_ident[1]), "ELF", 3) != 0)
  {
    Assert(0, "Not an ELF file!\n");
  }

  Assert(Elf_header.e_ident[EI_CLASS] == ELFCLASS32, "Not a 32-bit ELF file\n");
  Assert(Elf_header.e_type == ET_EXEC, "Not an exec file\n");

  // 移到.strtab的位置，并进行读取
  fseek(ftrace_fp, Elf_header.e_shoff + Elf_header.e_shentsize * (Elf_header.e_shstrndx - 1), SEEK_SET);
  ret = fread(&Elf_sec, Elf_header.e_shentsize, 1, ftrace_fp);
  if (ret != 1)
  {
    perror("Error reading from file");
  }
  str_off = Elf_sec.sh_offset;
  str_size = Elf_sec.sh_size;
  strtab = (char *)malloc(str_size);

  fseek(ftrace_fp, str_off, SEEK_SET);
  ret = fread(strtab, str_size, 1, ftrace_fp);
  if (ret != 1)
  {
    perror("Error reading from file");
  }

  // get .symtab
  for (int n = 0; n < Elf_header.e_shnum; n++)
  {
    fseek(ftrace_fp, Elf_header.e_shoff + n * Elf_header.e_shentsize, SEEK_SET);
    ret = fread(&Elf_sec, Elf_header.e_shentsize, 1, ftrace_fp);
    if (ret != 1)
    {
      perror("Error reading from file");
    }
    if (Elf_sec.sh_type == SHT_SYMTAB)
    {
      sym_off = Elf_sec.sh_offset;
      sym_size = Elf_sec.sh_entsize;
      sym_num = Elf_sec.sh_size / Elf_sec.sh_entsize;
      continue;
    }
  }

  return 0;
}

void free_strtab()
{
  free(strtab);
}
#endif

void device_update();

static void trace_and_difftest(vaddr_t dnpc){
  
#ifdef CONFIG_ITRACE_COND
  if (CONFIG_ITRACE_COND)
  {
    log_write("%s\n", s.logbuf);
  }
#endif
  if (g_print_step)
  {
    IFDEF(CONFIG_ITRACE, puts(s.logbuf));
  }

  bool success = true;
  uint32_t addr = 0;

  WP *index = head;
  while (index != NULL){
    addr = expr(index->target, &success);
    Assert(success,"Make_token fail!");

    if(addr != index->data){
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

#ifdef CONFIG_DIFFTEST
  for(int i = 0; i < RISCV_GPR_NUM; i ++){
    cpu.gpr[i] = R(i);
  }
 IFDEF(CONFIG_DIFFTEST, difftest_step(diff.pc, dnpc));
#endif

 

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

void inst_get(int inst){
  s.isa.inst.val = inst;
  // printf("s.isa.inst.val:0x%08x\n",s.isa.inst.val);
  // printf("inst:0x%08x\n",inst);
  // printf("get inst! \n");
}

#ifndef CONFIG_ISA_loongarch32r
void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
#endif

#ifdef CONFIG_FTRACE
struct func_call
{
  char *func_name;
  struct func_call *next;
  struct func_call *past;
};
#endif

/* let CPU conduct current command and renew PC */
static void exec_once()
{
  //上升沿取指令
  // if(dut.clk == 1) {
  //   if(dut.memWrite == 1) {
  //     printf("memWrite\n");
  //     paddr_write(dut.ALUResult,dut.DataLen + 1,dut.storeData);
  //   } 
  //   // printf("common:pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);
  // }
  // if(dut.memToReg == 1){
	// 		dut.ReadData = paddr_read(dut.ALUResult,dut.DataLen + 1);
	// 		dut.eval();
	// }

  // if(dut.clk == 1) dut.inst = paddr_read(dut.pc,4);
  // dut.eval();  
	// m_trace->dump(sim_time);
	// sim_time++;

  dut.clk ^= 1;
  dut.eval();
  #ifdef CONFIG_WAVE
  m_trace->dump(sim_time);
	sim_time++;
  #endif
		
  if(dut.invalid == 1){
    invalid_inst(dut.pc);
  }

  if(ifbreak && dut.clk == 0){
    printf("\nebreak!\n");
    // printf("ebreak: pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);
    NPCTRAP(dut.pc, 0);
  }
  if(dut.clk == 0 && !ifbreak)    return; 

  s.pc = dut.pc;
  s.snpc = s.pc + 4;
  s.dnpc = dut.rootp->ysyx_22041211_top__DOT__pc_next;
  cpu.pc = s.pc;

  #ifdef CONFIG_ITRACE
  char *p = s.logbuf;
  p += snprintf(p, sizeof(s.logbuf), FMT_WORD ":", s.pc);
  int ilen = s.snpc - s.pc;
  int i;
  uint8_t *inst = (uint8_t *)&s.isa.inst.val;

  for (i = ilen - 1; i >= 0; i--)
  {
    p += snprintf(p, 4, " %02x", inst[i]);
  }
  int ilen_max = MUXDEF(CONFIG_ISA_x86, 8, 4);
  int space_len = ilen_max - ilen;
  if (space_len < 0)
    space_len = 0;
  space_len = space_len * 3 + 1;
  memset(p, ' ', space_len);
  p += space_len;

#ifndef CONFIG_ISA_loongarch32r
  disassemble(p, s.logbuf + sizeof(s.logbuf) - p,
              MUXDEF(CONFIG_ISA_x86, s.snpc, s.pc), (uint8_t *)&s.isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif
#endif

#ifdef CONFIG_FTRACE
  //   static int j = 0;
  // if(j < 100){
  // 根据指令判断函数调用/函数返回

  // 1.把指令展开 放入一个char数组 12 13 15 16 18 19 21 22
  int k = 12;
  char *ins_tmp_16 = (char *)malloc(9);
  memset(ins_tmp_16, 0, 9);
  char *ins = (char *)malloc(33);
  memset(ins, 0, 33);

  // 1.1将logbuf中的指令存入临时数组
  for (int n = 0; n < 8; n++)
  {
    ins_tmp_16[n++] = s.logbuf[k++]; // 0 12 //2 15 //4 //6
    ins_tmp_16[n] = s.logbuf[k];     // 1 13 //3 16 //5 //7
    k += 2;
  }
  ins_tmp_16[8] = '\0';

  // 1.2将十六进制形式的指令转换为二进制
  // 1.2.1 转换为二进制形式
  // 8067 -> 0000 0000 0000 0000 1000 0000 0110 0111
  for (int n = 0; n < 8; n++)
  {
    char *per = convertTo_2(ins_tmp_16[n]);
    strcat(ins, per);
    free(per);
    per = NULL;
  }
  ins[32] = '\0';

  free(ins_tmp_16);

  // 2.判断函数调用/函数返回
  uint32_t m = s.isa.inst.val;
  bool if_return = false;
  bool if_conduct = false;

  // 函数返回 jalr, rd = x0, rs1 = x1, imm = 0
  // 函数调用 jal,  rd = x1, imm = ***
  // 函数调用 jalr, rd = x1, rs1 = a5, imm = 0
  // 函数调用 jalr, rd = x0, rs1 = a5, imm = 0

  // opcode rd rs1
  char *opcode = (char*)malloc(8);
  memset(opcode, 0, 8);
  strncpy(opcode, &ins[25], 7);
  opcode[7] = '\0';
  int rd = BITS(m, 11, 7);
  int rs1 = BITS(m, 19, 15);

  // 2.1 jal or jalr

  // 2.1.1 jal  函数调用 jal,  rd = x1, imm = ***
  if (strcmp(opcode, "1101111") == 0 && rd == 1)
  {
    if_return = false;
    if_conduct = true;
  }

  // 2.1.2 jalr 函数调用 or 函数返回
  // 函数返回 jalr, rd = x0, rs1 = x1, imm = 0
  // 函数调用 jalr, rd = x1, rs1 = a5, imm = 0
  // 函数调用 jalr, rd = x0, rs1 = a5, imm = 0

  // 判断出jalr
  else if (strcmp(opcode, "1100111") == 0)
  {

    // 函数返回
    if (rd == 0 && rs1 == 1)
    {
      if_return = true;
      if_conduct = true;
    }
    // 函数调用
    else if (rd == 1)
    {
      if_return = false;
      if_conduct = true;
    }
    else if (rd == 0)
    {
      if_return = false;
      if_conduct = true;
    }
  }

  if (if_conduct)
  {
    // 3.找到是哪个函数
    Elf32_Sym sym;
    int ret = 0;
    char *name = (char *)malloc(20);
    memset(name, 0, 20);

    for (int n = sym_num - 1; n >= 0; n--)
    {
      // 3.1读取符号表
      fseek(ftrace_fp, sym_off + n * sym_size, SEEK_SET);
      ret = fread(&sym, sizeof(Elf32_Sym), 1, ftrace_fp);
      if (ret != 1)
      {
        perror("Read error");
      }

      // 3.2找到对应的一行
      // 3.2.1 函数返回 是返回到原函数的中间位置
      if (if_return && (sym.st_value <= s.pc && sym.st_value + sym.st_size >= s.pc) && sym.st_info == 18)
      {
        // printf("sym.st_value = 0x%08x sym.st_size = %d \n",sym.st_value,sym.st_size);
        break;
      }
      // 3.2.2 函数调用 是跳转到一个新函数的头部
      else if (!if_return && sym.st_value == s.dnpc && sym.st_info == 18)
        break;
      if (n == 0)
      {
        Assert(0, "Fail in searching!");
      }
    }
 
    // 取出函数名称
    strncpy(name, strtab + sym.st_name, 19);

    // 4.调用的函数放入一个数据结构，返回函数放入一个数据结构
    static int index = 1;
    struct func_call *func;
    static struct func_call *func_cur = NULL;

    if (!if_return)
    {
      // 函数调用，将函数名放入链表
      func = (func_call*)malloc(sizeof(struct func_call));
      func->func_name = (char *)malloc(20);
      strcpy(func->func_name, name);
      func->past = func_cur;
      func->next = NULL;
      if (!func_cur)
      {
        func_cur = func;
      }
      else
      {
        func_cur->next = func;
        func_cur = func;
      }
      printf("index %d-> 0x%08x: \033[102m call[%s@0x%08x] \033[m\n", index, dut.pc, name, s.dnpc);
      index++;
    }
    else
    {
      // 函数返回，将函数名所在链表节点抽出
      while (1)
      {
        Assert(func_cur, "func_cur NULL!");
        int flag = 0;
        if (strcmp(func_cur->func_name, name) == 0)
          flag = 1;
        printf("index %d-> 0x%08x: \033[106m ret [%s] \033[m\n", index, dut.pc, func_cur->func_name);
        index++;

        free(func_cur->func_name);

        // 抽出节点
        if (func_cur->past == NULL)
        {
          free(func_cur);
        }
        else
        {
          func_cur = func_cur->past;
          free(func_cur->next);
          func_cur->next = NULL;
        }

        if (flag)
          break;
      }
    }
    Assert(func_cur, "func_cur NULL!");
    free(name);
  }
  free(opcode);
  free(ins);

#endif

}

/* stimulate the way CPU works ,get commands constantly */
static void execute(uint64_t n)
{
  for (; n > 0; n--)
  {
    exec_once();
    if(dut.clk == 1) g_nr_guest_inst++;  //记录客户指令的计时器

    if(dut.clk == 1) {
      // printf("cpu.pc = 0x%08x dut.pc = 0x%08x s.npc = 0x%08x\n",cpu.pc,dut.pc,s.dnpc);
      // printf("inst = 0x%08x\n",dut.rootp->ysyx_22041211_top__DOT__inst);
      trace_and_difftest(diff.dnpc);
    }

    diff.pc = s.pc;
    diff.snpc = s.snpc;
    diff.dnpc = s.dnpc;
    
    //当npc_state.state被设置为NPC_STOP时，npc停止执行指令
    if (npc_state.state != NPC_RUNNING)
      break;
    IFDEF(CONFIG_DEVICE, device_update());
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

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n)
{
  // g_print_step = (n < MAX_INST_TO_PRINT);
  switch (npc_state.state){
			case NPC_END:
			case NPC_ABORT:
				printf("Program execution has ended. To restart the program, exit NPC and run again.\n");
				dut.final();
        #ifdef CONFIG_WAVE
        m_trace->close();	//关闭波形跟踪文件
        #endif
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
    printf("\033[105m 0x%08x: \033[0m \t0x%08x\n",addr + i,paddr_read(addr + i,4));
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
#define AUTO_Q 1
/* Receive commands from user. */
void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    //return;
  }
  #ifdef AUTO_Q
  char arr[2] = {'a','b'};
  cmd_q(arr);
  return;

  #endif
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



