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

#include "cpu.h"
#include <locale.h>
#include "sdb.h"
#include <elf.h>

#include "isa.h"


/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;

#ifdef CONFIG_FTRACE
int init_ftrace(const char *ftrace_file){
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
    perror("Error reading from file");
  }

  //get .symtab
  for(int n = 0; n < Elf_header.e_shnum; n ++){
    fseek(ftrace_fp,Elf_header.e_shoff + n * Elf_header.e_shentsize,SEEK_SET);
    ret = fread(&Elf_sec,Elf_header.e_shentsize,1,ftrace_fp);
    if (ret != 1) {
      perror("Error reading from file");
    }
    if(Elf_sec.sh_type == SHT_SYMTAB){
      sym_off = Elf_sec.sh_offset;
      sym_size = Elf_sec.sh_entsize;
      sym_num = Elf_sec.sh_size / Elf_sec.sh_entsize;
      continue;
    }
  }

  return 0;
}

void free_strtab(){
  free(strtab);
}
#endif

void init_iringbuf(){
  int i;
  for(i = 0; i < 12; i ++){
    irbuf[i].next = (i == 11? irbuf: &irbuf[i + 1]);
    irbuf[i].past = (i == 0? &irbuf[11]: &irbuf[i - 1]);
  }
  header = irbuf;
  curre = irbuf;
  bottom = irbuf + 11;
}

iringbuf* get_head_iringbuf(){
  return irbuf;
}


typedef struct watchpoint {
  int NO;
  int times;
  uint32_t data;
  char target[100];
  struct watchpoint *next;
  struct watchpoint *past;

  /* TODO: Add more members if necessary */

} WP;

WP* get_head();

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

  WP *index = get_head();
  while (index != NULL)
  {
    addr = expr(index->target, &success);
    Assert(success,"Make_token fail!");
    if(addr != index->data){
      nemu_state.state = NEMU_STOP;
      index->times += 1;
      printf("\n\033[105m Hardware watchpoint %d: %s \033[0m\n", index->NO, index->target);
      printf("Old value = %d\n", index->data);
      printf("New value = %d\n\n", addr);
      index->data = addr;
      return;
    }
    index = index->next;
    return;

  }
  #endif
}

uint32_t convert_16(char *args);

//将十六进制数的每个十六进制位（数字或字母）转换为对应的四位二进制数
char *convertTo_2(char args){
  char *result = malloc(5);
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

#ifdef CONFIG_FTRACE
struct func_call{
  char *func_name;
  struct func_call *next;
  struct func_call *past;
};
#endif

/* let CPU conduct current command and renew PC */
static void exec_once(vaddr_t pc)
{
  s->pc = pc;
  s->snpc = pc;
  cpu.pc = s->dnpc;

}

/* stimulate the way CPU works ,get commands constantly */
static void execute(uint64_t n)
{
  for (; n > 0; n--)
  {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst++;  //记录客户指令的计时器
    if (nemu_state.state != NEMU_RUNNING)
      break;
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

void iringbuf_display(){
  for(int i = 0;i <12 ; i++){
    Assert(&(irbuf[i]) != NULL,"irbuf exists NULL!");
    if(irbuf[i].rbuf == NULL) break;
    if(irbuf + i == curre->past){
      printf(" --->\t%s\n",irbuf[i].rbuf);
      continue;
    }
    printf("\t%s\n",irbuf[i].rbuf);
  }
}

void assert_fail_msg()
{
  iringbuf_display();
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
        (nemu_state.state == NEMU_ABORT ? ANSI_FMT("ABORT", ANSI_FG_RED) : (nemu_state.halt_ret == 0 ? ANSI_FMT("HIT GOOD TRAP", ANSI_FG_GREEN) : ANSI_FMT("HIT BAD TRAP", ANSI_FG_RED))),
        nemu_state.halt_pc);
    // fall through
  case NEMU_QUIT:
    statistic();
  }
}
