#include <cpu/decode.h>
#include <isa.h>
#include <elf.h>
#include "reg.h"
#include <cpu/cpu.h>
#include <locale.h>
#include "sdb.h"

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
extern TOP_NAME dut;
CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
bool ifbreak = false;


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

#ifdef CONFIG_WATCHPOINT
extern WP *head;
#endif

void device_update();

void ifebreak_func(int inst){
	// printf("while key = 0x%08x\n",inst);printf("ebreak-called: pc = 0x%08x inst = 0x%08x\n",dut.pc,dut.inst)
	if(inst == 1048691) {ifbreak = true; } 
}

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
#ifdef CONFIG_WATCHPOINT
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
#endif
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

    //由于rtl对reg的更改是在下一个时钟周期上升沿，而nemu对reg的更改是即时的
    //所以这里要整个往后延迟一个周期
    if(cpu.pc != 0x80000000 && dut.clk == 1) {
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