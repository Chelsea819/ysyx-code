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

#include <assert.h>
#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <sdb.h>
#include <elf.h>

#include <isa.h>
#include "debug.h"
#include "reg.h"

#define OPCODE_JAL  0b1101111
#define OPCODE_JALR 0b1100111
/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

#ifdef CONFIG_IRINGBUF
typedef struct iringbuf_state
{
  char *rbuf;
  struct iringbuf_state *next;
  struct iringbuf_state *past;
} iringbuf;

iringbuf irbuf[12] = {};
static iringbuf *header = NULL;
static iringbuf *curre = NULL;
static iringbuf *bottom = NULL;
#endif

#ifdef CONFIG_WATCHPOINT
extern WP *head;
#endif

#ifdef CONFIG_FTRACE

typedef struct elf_table{
  char *name;
  int NO;
  FILE *ftrace_fp;
  Elf32_Ehdr Elf_header;
  Elf32_Shdr Elf_sec;
  Elf32_Off sym_off;
  Elf32_Off str_off;
  Elf32_Sym Elf_sym;
  Elf32_Xword str_size;
  Elf32_Xword sym_size;
  int sym_num;
  char *strtab;
} ELF; 

#define FILE_NUM 5
static ELF *elf_header = NULL;
static ELF *elf_pool = NULL;
// static ELF *elf_cur = NULL;
static int fileNum = 0;

typedef struct ftrace_file{
  char *name;
  int NO;
  struct ftrace_file *next;
} Ftrace_file;


int init_ftrace(Ftrace_file *file_header, Ftrace_file *file_cur)
{
  FILE *fp = NULL;
  // 需要读取的文件数
  fileNum = file_cur->NO - file_header->NO;

  // 初始化链表
  elf_pool = malloc(fileNum * sizeof(ELF));
  elf_header = elf_pool;

  // 循环读取文件
  for(int indx = 0; indx < fileNum; indx ++){
    fp = NULL;
    printf("fileNum = %d\n",fileNum);
    elf_header[indx].name = file_header[indx].name;
    elf_header[indx].NO = file_header[indx].NO;
    // 检查文件是否能正常读取
    Assert(file_header[indx].name, "file_header[indx].name is NULL!\n");
    Assert(elf_header[indx].name, "ftrace_file is NULL!\n");

    fp = fopen(elf_header[indx].name, "r");
    Assert(fp, "Can not open '%s'", elf_header[indx].name);

    elf_header[indx].ftrace_fp = fp;

    // 读取ELF header
    int ret = fread(&(elf_header[indx].Elf_header), sizeof(Elf32_Ehdr), 1, elf_header[indx].ftrace_fp);
    if (ret != 1){
      perror("Error reading from file");
    }
      printf("Elf_header.e_ident: %s\n",elf_header[indx].Elf_header.e_ident);

    if (elf_header[indx].Elf_header.e_ident[0] != '\x7f' || memcmp(&(elf_header[indx].Elf_header.e_ident[1]), "ELF", 3) != 0){
      Assert(0, "Not an ELF file!\n");
    }

    Assert(elf_header[indx].Elf_header.e_ident[EI_CLASS] == ELFCLASS32, "Not a 32-bit ELF file\n");
    Assert(elf_header[indx].Elf_header.e_type == ET_EXEC, "Not an exec file\n");

    // 移到.strtab的位置，并进行读取
    fseek(elf_header[indx].ftrace_fp, elf_header[indx].Elf_header.e_shoff + elf_header[indx].Elf_header.e_shentsize * (elf_header[indx].Elf_header.e_shstrndx - 1), SEEK_SET);
    ret = fread(&elf_header[indx].Elf_sec, elf_header[indx].Elf_header.e_shentsize, 1, elf_header[indx].ftrace_fp);
    if (ret != 1){
      perror("Error reading from file");
    }
    elf_header[indx].str_off = elf_header[indx].Elf_sec.sh_offset;
    elf_header[indx].str_size = elf_header[indx].Elf_sec.sh_size;
    elf_header[indx].strtab = malloc( elf_header[indx].str_size);

    fseek(elf_header[indx].ftrace_fp, elf_header[indx].str_off, SEEK_SET);
    ret = fread(elf_header[indx].strtab, elf_header[indx].str_size, 1, elf_header[indx].ftrace_fp);
    if (ret != 1){
      perror("Error reading from file");
    }

    // get .symtab
    for (int n = 0; n < elf_header[indx].Elf_header.e_shnum; n++){
      fseek(elf_header[indx].ftrace_fp, elf_header[indx].Elf_header.e_shoff + n * elf_header[indx].Elf_header.e_shentsize, SEEK_SET);
      ret = fread(&elf_header[indx].Elf_sec, elf_header[indx].Elf_header.e_shentsize, 1, elf_header[indx].ftrace_fp);
      if (ret != 1){
        perror("Error reading from file");
      }
      if (elf_header[indx].Elf_sec.sh_type == SHT_SYMTAB){
        elf_header[indx].sym_off = elf_header[indx].Elf_sec.sh_offset;
        elf_header[indx].sym_size = elf_header[indx].Elf_sec.sh_entsize;
        elf_header[indx].sym_num = elf_header[indx].Elf_sec.sh_size / elf_header[indx].Elf_sec.sh_entsize;
        continue;
      }
    }
  }
  return 0;
}


void free_strtab()
{
  for(int i = 0; i < fileNum; i ++){
    free(elf_header[i].strtab);
  }
  
}
#endif


#ifdef CONFIG_IRINGBUF
void init_iringbuf(){
  int i;
  for (i = 0; i < 12; i++)
  {
    irbuf[i].next = (i == 11 ? irbuf : &irbuf[i + 1]);
    irbuf[i].past = (i == 0 ? &irbuf[11] : &irbuf[i - 1]);
  }
  header = irbuf;
  curre = irbuf;
  bottom = irbuf + 11;
}

iringbuf *get_head_iringbuf()
{
  return irbuf;
}
#endif

void device_update();

static void trace_and_difftest(Decode *_this, vaddr_t dnpc)
{
#ifdef CONFIG_ITRACE_COND
  if (CONFIG_ITRACE_COND)
  {
    log_write("%s\n", _this->logbuf);
  }
#endif
  if (g_print_step)
  {
    IFDEF(CONFIG_ITRACE, puts(_this->logbuf));
  }
  IFDEF(CONFIG_DIFFTEST, difftest_step(_this->pc, dnpc));
#ifdef CONFIG_WATCHPOINT
  bool success = true;
  uint32_t addr = 0;

  // WP *index = get_head();
  WP *index = head;
  while (index != NULL)
  {
    // printf("\033[92m %d \thw watchpoint \tkeep \ty \t [%s] \033[m \n", index->NO, index->target);
    addr = expr(index->target, &success);
    Assert(success, "Make_token fail!");
    if (addr != index->data)
    {
      nemu_state.state = NEMU_STOP;
      index->times += 1;
      printf("\n\033[105m Hardware watchpoint %d: %s \033[0m\n", index->NO, index->target);
      printf("Old value = 0x%08x\n", index->data);
      printf("New value = 0x%08x\n\n", addr);
      printf("\033[105m at pc = [0x%08x] \033[0m\n", dnpc);
      index->data = addr;
      return;
    }
    index = index->next;  
  }
#endif
  return;
}



#ifdef CONFIG_FTRACE

struct func_call
{
  char *func_name;
  struct func_call *next;
  struct func_call *past;
};
#endif

/* let CPU conduct current command and renew PC */
static void exec_once(Decode *s, vaddr_t pc)
{
  s->pc = pc;
  s->snpc = pc;
  // 取指和译码

  //更新了snpc dnpc
  isa_exec_once(s);
  cpu.pc = s->dnpc;

  //到此为止pc指向了本次执行的指令 snpc指向了静态npc dnpc指向了动态npc

#ifdef CONFIG_ITRACE
  char *p = s->logbuf;
  p += snprintf(p, sizeof(s->logbuf), FMT_WORD ":", s->pc);
  int ilen = s->snpc - s->pc;
  int i;
  uint8_t *inst = (uint8_t *)&s->isa.inst.val;
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
  void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
  disassemble(p, s->logbuf + sizeof(s->logbuf) - p,
              MUXDEF(CONFIG_ISA_x86, s->snpc, s->pc), (uint8_t *)&s->isa.inst.val, ilen);
#else
  p[0] = '\0'; // the upstream llvm does not support loongarch32r
#endif

#endif

#ifdef CONFIG_FTRACE
  // 根据指令判断函数调用/函数返回

  // printf("s->isa.inst.val = 0x%08x\n",s->isa.inst.val);

  // 1.判断函数调用/函数返回
  uint32_t m = s->isa.inst.val;
  bool if_return = false;
  bool if_conduct = false;
  // bool if_recursion = false;

  // opcode rd rs1
  uint32_t opcode = BITS(m, 6, 0);
  uint32_t rd = BITS(m, 11, 7);
  uint32_t rs1 = BITS(m, 19, 15);

  // 2.1 jal or jalr

  // 2.1.1 jal  函数调用 jal,  rd = x1, imm = ***
  if (opcode == OPCODE_JAL && (rd == 1 || rd == 5)){
    if_return = false;
    if_conduct = true;
  }

  // 2.1.2 jalr 函数调用 or 函数返回

  // 判断出jalr
  else if (opcode == OPCODE_JALR){
    // 函数返回 jalr rs1 = x1, rd = x0 POP
    if ((!(rd == 1 || rd == 5)) && (rs1 == 1 || rs1 == 5)){
      if_return = true;
      if_conduct = true;
    }
    // 函数调用 PUSH
    else if ((rd == 1 || rd == 5) && (!(rs1 == 1 || rs1 == 5))){
      if_return = false;
      if_conduct = true;
    }
    else if ((rd == 1 || rd == 5) && (rs1 == 1 || rs1 == 5) && rd != rs1){
      // assert(0);
      // if_recursion = true;
      if_return = false;
      if_conduct = true;
    }
    else if ((rd == 1 || rd == 5) && (rs1 == 1 || rs1 == 5) && rd == rs1){
      if_return = false;
      if_conduct = true;
    }
  }

  if (if_conduct){
    // 3.找到是哪个函数
    Elf32_Sym sym;
    int ret = 0;
    int indx = 0;
    char *name = malloc(20);
    memset(name, 0, 20);

    for(indx = 0; indx < fileNum; indx ++){
      int n = elf_header[indx].sym_num - 1;
      for(; n >= 0; n--){
        // 3.1读取符号表
        fseek(elf_header[indx].ftrace_fp, elf_header[indx].sym_off + n * elf_header[indx].sym_size, SEEK_SET);
        ret = fread(&sym, sizeof(Elf32_Sym), 1, elf_header[indx].ftrace_fp);
        if (ret != 1){
          perror("Read error");
        }
        // strncpy(name, elf_header[indx].strtab + sym.st_name, 19); printf("name: %s\n",name);
        // 3.2找到对应的一行
        // 3.2.1 函数返回 是返回到原函数的中间位置
        if (if_return && (sym.st_value <= s->pc && sym.st_value + sym.st_size >= s->pc) && sym.st_info == 18){
          // printf("sym.st_value = 0x%08x sym.st_size = %d \n",sym.st_value,sym.st_size);
          break;
        }
        // 3.2.2 函数调用 是跳转到一个新函数的头部
        else if (!if_return && sym.st_value == s->dnpc && sym.st_info == 18){
          // printf("sym.st_value = 0x%08x s->dnpc = 0x%08x \n",sym.st_value,s->dnpc);
          break;
        }
      }
      if(n >= 0) break;
      if (indx == fileNum - 1){
          Assert(0, "Fail in searching!");
      }
    }
  
    // 取出函数名称
    strncpy(name, elf_header[indx].strtab + sym.st_name, 19);
    // 4.调用的函数放入一个数据结构，返回函数放入一个数据结构
    static int index = 1;

    if (if_return){
      // 函数返回，将函数名所在链表节点抽出
        Assert(name, "name NULL!");
        if(strcmp(name,"putch") != 0) printf("index %d-> 0x%08x: \033[106m ret [%s] \033[m\n", index, cpu.pc, name);
        index++;
    }
    else{
      // 函数调用，将函数名放入链表
      if(strcmp(name,"putch") != 0) printf("index %d-> 0x%08x: \033[102m call[%s @0x%08x] \033[m\n", index, cpu.pc, name, s->dnpc);
      #ifdef CONFIG_FTRACE_PASS
      if(strcmp(name,"putch") != 0) 
        for(int i = 10; i < 15; i++){
          printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,regs[i],gpr(i));
        }
      #endif
      index++;
    }
    free(name);
  }

#endif

#ifdef CONFIG_IRINGBUF
  if (curre == header && curre->rbuf != NULL)
  {
    header = header->next;
    bottom = curre;
  }
  else
  {
    curre->rbuf = malloc(sizeof(char) * 50);
  }

  #ifndef CONFIG_ITRACE
    char ir_logbuf[128] = {0};
    char *ir = ir_logbuf;
    ir += snprintf(ir, sizeof(ir_logbuf), FMT_WORD ":", s->pc);
    int ilen = s->snpc - s->pc;
    int i;
    uint8_t *inst_ir = (uint8_t *)&s->isa.inst.val;

    for (i = ilen - 1; i >= 0; i--)
    {
      ir += snprintf(ir, 4, " %02x", inst_ir[i]);
    }
    strcpy(curre->rbuf, ir_logbuf);
  
  #else

    strcpy(curre->rbuf, s->logbuf);

  #endif

  curre = curre->next;
#endif
}

/* stimulate the way CPU works ,get commands constantly */
static void execute(uint64_t n)
{
  // watchPoints_display();
  Decode s;
  for (; n > 0; n--)
  {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst++; // 记录客户指令的计时器
    trace_and_difftest(&s, cpu.pc);
    // 当nemu_state.state被设置为NEMU_STOP时，nemu停止执行指令
    if (nemu_state.state != NEMU_RUNNING)
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

#ifdef CONFIG_IRINGBUF
void iringbuf_display(){
  for (int i = 0; i < 12; i++)
  {
    Assert(&(irbuf[i]) != NULL, "irbuf exists NULL!");
    if (irbuf[i].rbuf == NULL)
      break;
    if (irbuf + i == curre->past)
    {
      printf(" --->\t%s\n", irbuf[i].rbuf);
      continue;
    }
    printf("\t%s\n", irbuf[i].rbuf);
  }
}
#endif

void assert_fail_msg()
{
#ifdef CONFIG_IRINGBUF
  iringbuf_display();
#endif
  isa_reg_display();
  statistic();
}

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n)
{
  g_print_step = (n < MAX_INST_TO_PRINT);
  switch (nemu_state.state)
  {
  case NEMU_END:
  case NEMU_ABORT:
    printf("Program execution has ended. To restart the program, exit NEMU and run again.\n");
    return;
  default:
    nemu_state.state = NEMU_RUNNING;
  }

  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  switch (nemu_state.state)
  {
  case NEMU_RUNNING:
    nemu_state.state = NEMU_STOP;
    break;

  case NEMU_END:
  case NEMU_ABORT:
    Log("nemu: %s at pc = " FMT_WORD,
        (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : 
        (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
        nemu_state.halt_pc);
    // fall through
  case NEMU_QUIT:
    statistic();
  }
}
