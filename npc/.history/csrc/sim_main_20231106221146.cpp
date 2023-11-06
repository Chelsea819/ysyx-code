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

#include <readline/readline.h>
#include <readline/history.h>
#include <locale.h>
#include <time.h>
#include <getopt.h>
#include <regex.h>
#include "utils.h"
#include "common.h"

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

TOP_NAME dut;
VerilatedVcdC *m_trace = new VerilatedVcdC;


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
  0x00100093,  // addi    x[1] = 0 + 1
  0x00000297,  // auipc t0,0
  0x00100073,  // ebreak (used as npc_trap)
  0x00100093,  // addi    x[1] = 0 + 1
};

void init_rand() {
  srand(time(0));
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



static word_t pmem_read_npc(paddr_t addr,int len) {
  word_t ret = host_read_npc(guest_to_host_npc(addr), len);
  return ret;
}

static void pmem_write_npc(paddr_t addr, int len, word_t data) {
  host_write_npc(guest_to_host_npc(addr), len, data);
}


static vaddr_t load_mem_npc(paddr_t addr,int len) {
	return pmem_read_npc(addr,len);
}

void ifebreak_func(int inst){
	// printf("while key = 0x%08x\n",inst);
	if(inst == 1048691) {ifbreak = true; printf("ebreak-called: pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst);} 
}

void mem_write_npc(vaddr_t addr, int len, word_t data) {
  pmem_write_npc(addr, len, data);
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

void init_sdb()
{
  /* Compile the regular expressions. */
  // 编译正则表达式
  init_regex();
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
  temp[0] = pmem_read_npc(pc, 4);
  temp[1] = pmem_read_npc(pc, 4);

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

/* let CPU conduct current command and renew PC */
static void exec_once(vaddr_t pc)
{
  //上升沿取指令
  if(dut.clk == 1) {
    if(dut.memWrite) mem_write_npc(dut.ALUResult,dut.DataLen + 1,dut.storeData);
    dut.inst = pmem_read_npc(dut.pc,4);
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
    // trace_and_difftest(&s, cpu.pc);
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
        exit(EXIT_SUCCESS);
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


/* ------------------------------------sdb------------------------------------ */

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
  cpu_exec(n);
  return 0;
}

static int cmd_q(char *args)
{
  npc_state.state = NPC_QUIT;
  return -1;
}

static int cmd_help(char *args);



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
    // {"info","Print the state of register(r) or the content of watchpoint(w)",cmd_info},
    // {"p","Get the result of EXPR.",cmd_pcount},
    // {"x","Scan the memory.",cmd_x},
    // {"w","Set a watchpoint.",cmd_w},
    // {"d","Delete certain watchpoint.",cmd_d},

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
	// 		dut.inst = pmem_read_npc(dut.pc,4);
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



