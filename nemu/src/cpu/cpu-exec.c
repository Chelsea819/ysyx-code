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
  int sym_num;
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
    if(Elf_sec.sh_type == SHT_SYMTAB){
      printf("Elf_sec.sh_name = %d\n",Elf_sec.sh_name);
      sym_off = Elf_sec.sh_offset;
      sym_size = Elf_sec.sh_entsize;
      sym_num = Elf_sec.sh_size / Elf_sec.sh_entsize;
      continue;
    }
  }
  
  //读取.symtab
  fseek(ftrace_fp,sym_off,SEEK_SET);
  ret = fread(&Elf_sym,sizeof(Elf32_Sym),1,ftrace_fp);
  if (ret != 1) {
    perror("Error reading from file");
  }

  printf("sym_off = %d\n",sym_off);
  printf("sym_size = %ld sym_num = %d\n",sym_size,sym_num);
  
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

const char *reg[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

//uint32_t convert_16(char *args);

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

/* let CPU conduct current command and renew PC */
static void exec_once(Decode *s, vaddr_t pc)
{
  s->pc = pc;
  s->snpc = pc;
  isa_exec_once(s);
  cpu.pc = s->dnpc;
  //printf("s->logbuf = %s\n",s->logbuf);
  

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
    static int j = 0;
  if(j < 10){
  //根据指令判断函数调用/函数返回

  //1.把指令展开 放入一个char数组 12 13 15 16 18 19 21 22
  int k = 12;
  char *ins_tmp_16 = malloc(9);
  char *ins = malloc(33);
  
  //1.1将logbuf中的指令存入临时数组
  for(int n = 0; n < 8; n ++){
    ins_tmp_16[n++] = s->logbuf[k++]; //0 12 //2 15 //4 //6
    ins_tmp_16[n] = s->logbuf[k];     //1 13 //3 16 //5 //7
    k += 2;
  }
  ins_tmp_16[8] = '\0';
  printf("ins_tmp_16 = _%s_\n",ins_tmp_16);

  // 1.2将十六进制形式的指令转换为二进制
  // 1.2.1 转换为二进制形式
  // 8067 -> 0000 0000 0000 0000 1000 0000 0110 0111
  memset(ins,0,33);
  for(int n = 0; n < 8; n ++){
    char *per = convertTo_2(ins_tmp_16[n]);
    strcat(ins,per);
    free(per);
    per = NULL;
  } 
  ins[32] = '\0';
  printf("ins = %s\n",ins);
  printf("s->logbuf = %s\n",s->logbuf);

  free(ins);
  free(ins_tmp_16);

  //2.判断函数调用/函数返回

  //函数返回 jalr, rd = x0, rs1 = x1, imm = 0
  //函数调用 jal,  rd = x1, imm = ***
  //函数调用 jalr, rd = x1, rs1 = a5, imm = 0
  //函数调用 jalr, rd = x0, rs1 = a5, imm = 0

  //3.取出跳转地址

  //4.找到是哪个函数

  //5.调用的函数放入一个数据结构，返回函数放入一个数据结构
  
  // char addr_tmp[11] = {0};
  // int addr = 0;
  // char reg_tmp[3] = {0};
  // char reg_tmp_zero[3] = {0};
  // char *name = malloc(20);
  // static int n = 0;
  // bool if_return = false;
  // bool if_conduct = true;
  // int ret = 0;
  // Elf32_Sym sym;
  // //检测jalr函数调用/函数返回，取出跳转到的地址 
  // if(strncmp(&(s->logbuf[24]),"jalr",strlen("jalr")) == 0){
  //   strncpy(reg_tmp,&(s->logbuf[35]),2);
  //   strncpy(reg_tmp_zero,&(s->logbuf[37]),2);
  //   //printf("reg = %s\n",reg_tmp);
  //   for(int i = 0; i < 32; i ++){
  //     if(strncmp(reg[i],reg_tmp,strlen("ra")) == 0){
  //       addr = gpr(i);
  //       break;
  //     }
  //     //返回函数
  //     if(strncmp(reg[i],reg_tmp_zero,strlen("ra")) == 0){
  //       addr = gpr(i);
  //       if(strncmp(reg[i],"ra",strlen("ra")) == 0)  if_return = true;
  //       break;
  //     }
  //     if(i == 31) Assert(0,"Fail in get reg!");
  //   }
  // } 
  // //检测jal 函数调用 取出跳转到的地址
  // else if(strncmp(&(s->logbuf[24]),"jal",strlen("jal")) == 0){
  //   strncpy(addr_tmp,&(s->logbuf[32]),10);
  //   addr = convert_16(addr_tmp);
  //   //printf("addr_tmp = %s  addr = 0x%08x\n",addr_tmp,addr);
  // }
  // else{
  //   if_conduct = false;
  // }  
  // //将地址与函数对应
  // if(if_conduct){
  //   printf("s->logbuf: %s\n",s->logbuf);
  //   for(int n = 0; n < sym_num; n ++){
  //     fseek(ftrace_fp,sym_off + n * sym_size,SEEK_SET);
  //     ret = fread(&sym,sizeof(Elf32_Sym),1,ftrace_fp);
  //     if(ret != 1){
  //       perror("Read error");
  //     }
  //     if(if_return && (sym.st_value <= addr && sym.st_value + sym.st_size >= addr )&& sym.st_info == 18){
  //       //printf("sym.st_value = 0x%08x sym.st_size = %d \n",sym.st_value,sym.st_size);
  //       break;
  //     }
  //     else if(!if_return && sym.st_value == addr && sym.st_info == 18) break;
  //     if(n == sym_num - 1){
  //       Assert(0,"Fail in searching!");
  //     }
  //   }

  //   //printf("st_name: 0x%08x ",sym.st_name);

  //   //读出来的函数名不对
  //   n++;
  //   strncpy(name,strtab + sym.st_name,19);
  //   if(!if_return) printf("\033[102m %d:  0x%08x: call[%s@0x%08x] \033[m\n",n,cpu.pc,name,addr);
  //   else printf("\033[102m %d:  0x%08x: ret [%s] \033[m\n",n,cpu.pc,name);
  // }

  j ++;
  }

  if(curre == header && curre->rbuf != NULL){
    header = header->next;
    bottom = curre;
  }
  else{
    curre->rbuf = malloc(sizeof(char) * 50);
  }
  strcpy(curre->rbuf,s->logbuf);
  curre = curre->next;

  // if(n < 19){
  //   printf("s->logbuf: %s\n",s->logbuf);
  //   n ++;
  // }

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
