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

#include <cpu/cpu.h>
#include <cpu/decode.h>
#include <cpu/difftest.h>
#include <locale.h>
#include <sdb.h>
#include <elf.h>

#include <isa.h>
#include "../isa/riscv32/local-include/reg.h"

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

#ifdef CONFIG_FTRACE
FILE *ftrace_fp = NULL;

Elf32_Ehdr Elf_header;
Elf32_Shdr Elf_sec;
Elf32_Off sym_off;
Elf32_Off str_off;
Elf32_Sym Elf_sym;
Elf32_Xword str_size;
Elf32_Xword sym_size;
int sym_num;
extern WP *head;
char *strtab = NULL;


int init_ftrace(const char *ftrace_file)
{
  FILE *fp = NULL;

  // 检查文件是否能正常读取
  Assert(ftrace_file, "ftrace_file is NULL!\n");

  fp = fopen(ftrace_file, "r");
  Assert(fp, "Can not open '%s'", ftrace_file);

  ftrace_fp = fp;

  // 读取ELF header
  int ret = fread(&Elf_header, sizeof(Elf32_Ehdr), 1, ftrace_fp);
  if (ret != 1){
    perror("Error reading from file");
  }
  if (Elf_header.e_ident[0] != '\x7f' || memcmp(&(Elf_header.e_ident[1]), "ELF", 3) != 0){
    Assert(0, "Not an ELF file!\n");
  }

  Assert(Elf_header.e_ident[EI_CLASS] == ELFCLASS32, "Not a 32-bit ELF file\n");
  Assert(Elf_header.e_type == ET_EXEC, "Not an exec file\n");

  // 移到.strtab的位置，并进行读取
  fseek(ftrace_fp, Elf_header.e_shoff + Elf_header.e_shentsize * (Elf_header.e_shstrndx - 1), SEEK_SET);
  ret = fread(&Elf_sec, Elf_header.e_shentsize, 1, ftrace_fp);
  if (ret != 1){
    perror("Error reading from file");
  }
  str_off = Elf_sec.sh_offset;
  str_size = Elf_sec.sh_size;
  strtab = malloc(str_size);

  fseek(ftrace_fp, str_off, SEEK_SET);
  ret = fread(strtab, str_size, 1, ftrace_fp);
  if (ret != 1){
    perror("Error reading from file");
  }

  // get .symtab
  for (int n = 0; n < Elf_header.e_shnum; n++){
    fseek(ftrace_fp, Elf_header.e_shoff + n * Elf_header.e_shentsize, SEEK_SET);
    ret = fread(&Elf_sec, Elf_header.e_shentsize, 1, ftrace_fp);
    if (ret != 1){
      perror("Error reading from file");
    }
    if (Elf_sec.sh_type == SHT_SYMTAB){
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
  if (ITRACE_COND)
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
  // assert(index != NULL);
  while (index != NULL){
    // printf("\033[92m %d \thw watchpoint \tkeep \ty \t [%s] \033[m \n", index->NO, index->target);
    addr = expr(index->target, &success);
    Assert(success, "Make_token fail!");
    if (addr != index->data){
      nemu_state.state = NEMU_STOP;
      index->times += 1;
      printf("\n\033[105m Hardware watchpoint %d: %s \033[0m\n", index->NO, index->target);
      printf("Old value = 0x%08x\n", index->data);
      printf("New value = 0x%08x\n\n", addr);
      index->data = addr;
      return;
    }
    index = index->next;  
  }
#endif
  return;
}

const char *reg[] = {
    "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
    "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
    "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
    "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"};

uint32_t convert_16(char *args);

// 将十六进制数的每个十六进制位（数字或字母）转换为对应的四位二进制数
char *convertTo_2(char args)
{
  char *result = malloc(5);
  int num = 0;

  if (args >= 'a' && args <= 'f')
  {
    num = (int)args - (int)'a' + 10;
  }
  else
  {
    num = (int)args - (int)'0';
  }

  int flag = 8;
  // 15 -> 8 + 4 + 2 + 1
  // 5 -> 4 + 1
  // 0
  for (int n = 0; n < 4; n++)
  {
    // 为0 则该位之后的低位均为0
    if (!num)
    {
      for (; n < 4; n++)
        result[n] = '0';
      break;
    }

    result[n] = num / flag + '0';
    if (num / flag)
      num -= flag;
    flag /= 2;
  }
  result[4] = '\0';
  return result;
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
  //   static int j = 0;
  // if(j < 100){
  // 根据指令判断函数调用/函数返回

  // 1.把指令展开 放入一个char数组 12 13 15 16 18 19 21 22
  // printf("s->logbuf = %s\n",s->logbuf);
  // printf("s->isa.inst.val = 0x%08x\n",s->isa.inst.val);
  int k = 12;
  char *ins_tmp_16 = malloc(9);
  memset(ins_tmp_16, 0, 9);
  char *ins = malloc(33);
  memset(ins, 0, 33);

  // 1.1将logbuf中的指令存入临时数组
  for (int n = 0; n < 8; n++)
  {
    ins_tmp_16[n++] = s->logbuf[k++]; // 0 12 //2 15 //4 //6
    ins_tmp_16[n] = s->logbuf[k];     // 1 13 //3 16 //5 //7
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
  uint32_t m = s->isa.inst.val;
  bool if_return = false;
  bool if_conduct = false;
  // bool if_same = false;
  // 函数返回 jalr, rd = x0, rs1 = x1, imm = 0
  // 函数调用 jal， x0 ,offset
  // 函数调用 jal,  rd = x1, imm = ***
  // 函数调用 jalr, rd = x1, rs1 = a5, imm = 0
  // 函数调用 jalr, rd = x0, rs1 = a5, imm = 0

  // opcode rd rs1
  char *opcode = malloc(8);
  memset(opcode, 0, 8);
  strncpy(opcode, &ins[25], 7);
  opcode[7] = '\0';
  int rd = BITS(m, 11, 7);
  int rs1 = BITS(m, 19, 15);
  // int a = 0;

  // 2.1 jal or jalr

  // 2.1.1 jal  函数调用 jal,  rd = x1, imm = ***
  if (strcmp(opcode, "1101111") == 0 && (rd == 1 || rd == 0)){
    if_return = false;
    if_conduct = true;
  }

  // 2.1.2 jalr 函数调用 or 函数返回
  // 函数返回 jalr, rd = x0, rs1 = x1, imm = 0
  // 函数调用 jalr, rd = x1, rs1 = a5, imm = 0
  // 函数调用 jalr, rd = x0, rs1 = a5, imm = 0

  // 函数返回 jalr rs1 = x1, rd = x0
  // 函数调用 jalr, rd = x0
  // 函数调用 jalr, rd = x1

  // 判断出jalr
  else if (strcmp(opcode, "1100111") == 0){

    // 函数返回 jalr rs1 = x1, rd = x0
    if (rd == 0 && rs1 == 1){
      if_return = true;
      if_conduct = true;
    }
    // 函数调用
    else if (rd == 1){
      if_return = false;
      if_conduct = true;
    }
    else if (rd == 0){
      if_return = false;
      if_conduct = true;
    }
  }

  if (if_conduct){
    // 3.找到是哪个函数
    Elf32_Sym sym;
    int ret = 0;
    char *name = malloc(20);
    memset(name, 0, 20);

    // printf("s->logbuf: %s\n",s->logbuf);

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
      if (if_return && (sym.st_value <= s->pc && sym.st_value + sym.st_size >= s->pc) && sym.st_info == 18)
      {
        // printf("sym.st_value = 0x%08x sym.st_size = %d \n",sym.st_value,sym.st_size);
        break;
      }
      // 3.2.2 函数调用 是跳转到一个新函数的头部
      else if (!if_return && sym.st_value == s->dnpc && sym.st_info == 18)
        break;
      if (n == 0){
        // if_same = true;
        Assert(0, "Fail in searching!");
      }
    }
    // if(!if_same){
      // 取出函数名称
      strncpy(name, strtab + sym.st_name, 19);
      

      // 4.调用的函数放入一个数据结构，返回函数放入一个数据结构

      static int index = 1;
      struct func_call *func;
      static struct func_call *func_cur = NULL;

      if(!if_return && func_cur != NULL && strcmp(name, func_cur->func_name) == 0){
        
      }
      else if (!if_return){
        // 函数调用，将函数名放入链表
        func = malloc(sizeof(struct func_call));
        func->func_name = malloc(20);
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
        if(strcmp(name,"putch") != 0) printf("index %d-> 0x%08x: \033[102m call[%s@0x%08x] \033[m\n", index, cpu.pc, name, s->dnpc);
        index++;
      }
      else{
        // 函数返回，将函数名所在链表节点抽出
        while (1){
          Assert(func_cur, "func_cur NULL!");
          Assert(func_cur->func_name, "func_cur->func_name NULL!");
          Assert(name, "name NULL!");
          int flag = 0;
          if (strcmp(func_cur->func_name, name) == 0)
            flag = 1;
          if(flag == 0) printf("name:%s\nfunc_cur->func_name:%s\n",name,func_cur->func_name);

          if(strcmp(name,"putch") != 0) printf("index %d-> 0x%08x: \033[106m ret [%s] \033[m\n", index, cpu.pc, func_cur->func_name);
          index++;
          // printf("name:%s\n",name);

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

          if (flag) break;

          printf("flag = %d\n",flag);
        }
      }
      // Assert(func_cur, "func_cur NULL!");
      free(name);
    // }
  }
  free(opcode);
  free(ins);

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
