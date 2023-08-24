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
#include "../monitor/sdb/sdb.h"
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

typedef struct iringbuf_state{
  char *rbuf;
  struct iringbuf_state *next;
  struct iringbuf_state *past;
}iringbuf;

iringbuf irbuf[12]={};
static iringbuf* header = NULL;
static iringbuf* curre = NULL;
static iringbuf* bottom = NULL;

FILE *ftrace_fp = NULL;

  Elf32_Ehdr Elf_header;
  Elf32_Shdr Elf_sec;
  Elf32_Off sym_off;
  Elf32_Off str_off;
  Elf32_Sym  Elf_sym;
  Elf32_Xword str_size;
  Elf32_Xword sym_size;
  char *strtab = NULL;

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
      sym_size = Elf_sec.sh_entsize;
      continue;
    }
  }
  
  //读取.symtab
  fseek(ftrace_fp,sym_off,SEEK_SET);
  ret = fread(&Elf_sym,sizeof(Elf32_Sym),1,ftrace_fp);
  if (ret != 1) {
    perror("Error reading from file");
  }

  printf(".strtab : _%s_  length = %ld\n",&strtab[9],strlen(&strtab[9]));
  printf("str_off = %d \n",str_off);
  printf("sym_off = %d\n",sym_off);
  printf("str_size = %ld\n",str_size);
  
  return 0;
}

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

void device_update();


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

// // 28 - 14 - 7 - 3 - 1 - 0
// //           1   1   1
// void convert_2(int num){
//   char result[100] = {0};
//   int tmp[100] = {0};
//   int n = 0;
//   for(n = 0; num / 2 != 0; n ++){
//     tmp[n] = num % 2;
//     num /= 2;
//   }
//   n --;
//   for( ; n < 32; n ++){
//     tmp[n] = 0;
//   }
//   for(int k = 0; n >= 0; n --, k ++){
//     result[k] = tmp[n] + '0';
//   }
  
//   printf("result = %s",result);
//   //return result;
// }

const char *reg[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

uint32_t convert_16(char *args);
static int n = 0;

/* let CPU conduct current command and renew PC */
static void exec_once(Decode *s, vaddr_t pc)
{
  //chat *iringbuf = 
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
  
  char addr_tmp[11] = {0};
  int addr = 0;
  char reg_tmp[3] = {0};
  char name[20] = {0};
  bool if_return = false;
  bool if_conduct = true;
  int ret = 0;
  Elf64_Sym sym;
  //检测jal 函数调用 取出跳转到的地址
  if(strncmp(&(s->logbuf[24]),"jal",strlen("jal")) == 0){
    strncpy(addr_tmp,&(s->logbuf[32]),10);
    addr = convert_16(addr_tmp);
    printf("addr_tmp = %s  addr = 0x%08x\n",addr_tmp,addr);
  } 
  //检测jalr函数调用/函数返回，取出跳转到的地址
  else if(strncmp(&(s->logbuf[24]),"jalr",strlen("jalr")) == 0){
    strncpy(reg_tmp,&(s->logbuf[37]),2);
    for(int i = 0; i < 32; i ++){
      if(strncmp(reg[i],reg_tmp,strlen("ra")) == 0){
        addr = gpr(i);
        break;
      }
    }
    //返回函数
    if(strncmp(reg_tmp,"ra",strlen("ra")) == 0){
      if_return = true;
    }
  }
  else{
    if_conduct = false;
  }  
  //将地址与函数对应
  printf("sym_size = %ld %ld\n",sym_size,sizeof(Elf32_Sym));
  if(if_conduct){
    for(int n = 0; ;n ++){
      fseek(ftrace_fp,sym_off + n * sym_size,SEEK_SET);
      ret = fread(&sym,sizeof(Elf32_Sym),1,ftrace_fp);
      printf("sym.st_value = 0x%08lx sym.st_size = %ld \n",sym.st_value,sym.st_size);
      if(ret != 1){
        perror("Read error");
      }
      if(sym.st_value == addr){
        break;
      }
      if(n == 50){
        Assert(0,"Fail in searching!");
      }
    }
    fseek(ftrace_fp,sym.st_name,SEEK_SET);
    ret = fread(name,19,1,ftrace_fp);
    if(if_return) printf("0x%08x: call[%s@0x%08x]\n",cpu.pc,name,addr);
    else printf("0x%08x: ret [%s]\n",cpu.pc,name);
  }
  

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

  if(n < 6){
    n ++;
  printf("val = %s len = %ld\n ",&(s->logbuf[24]),strlen(s->logbuf));}

  if(curre == header && curre->rbuf != NULL){
    header = header->next;
    bottom = curre;
  }
  else{
    curre->rbuf = malloc(sizeof(char) * 50);
  }
  strcpy(curre->rbuf,s->logbuf);
  curre = curre->next;

  

}

/* stimulate the way CPU works ,get commands constantly */
static void execute(uint64_t n)
{
  Decode s;
  for (; n > 0; n--)
  {
    exec_once(&s, cpu.pc);
    g_nr_guest_inst++;  //记录客户指令的计时器
    trace_and_difftest(&s, cpu.pc);
    //当nemu_state.state被设置为NEMU_STOP时，nemu停止执行指令
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
