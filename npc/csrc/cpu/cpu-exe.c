#include <cpu/decode.h>
#include <cstdint>
#include <cstdio>
#include <isa.h>
#include <elf.h>
#include "reg.h"
#include <cpu/cpu.h>
#include <locale.h>
#include <cpu/difftest.h>
#include <debug.h>
#include <config.h>
#include "sdb.h"

#define OPCODE_JAL  0b1101111
#define OPCODE_JALR 0b1100111

word_t expr(char *e, bool *success);
Decode s;

#ifdef CONFIG_DIFFTEST
// store the last pc
static struct diff_pc{
  paddr_t pc;
  paddr_t dnpc;
}diff;
#endif

extern vluint64_t sim_time;
extern TOP_NAME *dut; extern VerilatedVcdC *m_trace;
CPU_state cpu = {};
uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
bool ifbreak = false;

extern "C" void pc_get(int pc, int dnpc){
  // printf("pc = %x dpc = %x\n",pc,dnpc);
  cpu.pc = pc;
  # if (defined CONFIG_DIFFTEST) || (defined CONFIG_TRACE)
    s.pc = pc;
    s.dnpc = dnpc;
  #endif
}

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

struct func_call
{
  char *func_name;
  struct func_call *next;
  struct func_call *past;
};


int init_ftrace(Ftrace_file *file_header, Ftrace_file *file_cur)
{
  Assert(file_header != NULL && file_cur != NULL,"Pass file failed!");
  FILE *fp = NULL;
  // 需要读取的文件数
  fileNum = file_cur->NO - file_header->NO;

  // 初始化链表
  elf_pool = (ELF*)malloc(fileNum * sizeof(ELF));
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
    elf_header[indx].strtab = (char*)malloc( elf_header[indx].str_size);

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

#ifdef CONFIG_WATCHPOINT
extern WP *head;
#endif

void device_update();

void ifebreak_func(int inst){
	// printf("while key = 0x%08x\n",inst);printf("ebreak-called: pc = 0x%08x inst = 0x%08x\n",cpu.pc,dut->inst)
	if(inst == 1048691) {ifbreak = true; } 
}

static void trace_and_difftest(){
  
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
    // printf("index->target: %s, addr: 0x%08x, index->data: 0x%08x\n",index->target,addr,index->data);
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
  cpu.mcause = dut->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__ysyx_22041211_cpu__DOT__ysyx_22041211_CSR__DOT__csr[0];
  cpu.mepc = dut->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__ysyx_22041211_cpu__DOT__ysyx_22041211_CSR__DOT__csr[2];
  cpu.mstatus = dut->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__ysyx_22041211_cpu__DOT__ysyx_22041211_CSR__DOT__csr[1];
  cpu.mtvec = dut->rootp->ysyxSoCFull__DOT__asic__DOT__cpu__DOT__cpu__DOT__ysyx_22041211_cpu__DOT__ysyx_22041211_CSR__DOT__csr[3];
 IFDEF(CONFIG_DIFFTEST, difftest_step(diff.pc, diff.dnpc));
#endif

 

  return;
}

#ifdef CONFIG_FTRACE
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
#endif

void inst_get(int inst){
  s.isa.inst.val = inst;
  // printf("s.isa.inst.val:0x%08x\n",s.isa.inst.val);
  // printf("inst:0x%08x\n",inst);
  // printf("get inst! \n");
}

char inst_invalid = 0;
void inst_invalid_get(char invalid){
  inst_invalid = invalid;
  // printf("s.isa.inst.val:0x%08x\n",s.isa.inst.val);
  // printf("inst:0x%08x\n",inst);
  // printf("get inst! \n");
}

char inst_finish = 0;
void finish_get(char finish){
  inst_finish = finish;
  // printf("s.isa.inst.val:0x%08x\n",s.isa.inst.val);
  // printf("inst:0x%08x\n",inst);
  // printf("get inst! \n");
}


#ifndef CONFIG_ISA_loongarch32r
void disassemble(char *str, int size, uint64_t pc, uint8_t *code, int nbyte);
#endif

void per_clk_cycle(){
  do {
    dut->clock ^= 1;
    dut->eval();
    #ifdef CONFIG_WAVE
    m_trace->dump(sim_time);
    sim_time++;
    #endif
    
  }while(dut->clock == 1);
  // printf("clk = %d\n",dut->clock);
}
void per_inst_cycle(){
  // printf("dut.pc = [0x%08x]!\n",cpu.pc);
  int i = 20;
  do {
    // printf("dut.pc = [0x%08x]!\n",cpu.pc);
    per_clk_cycle(); i--;
    // printf("unfinshed!\n");
  }while(inst_finish == 0 && i > 0);
  // printf("finished dut.pc = [0x%08x]!\n",cpu.pc);
}



/* let CPU conduct current command and renew PC */
static void exec_once()
{
  per_inst_cycle();
  // inst invalid check
  if(inst_invalid != 0){
    invalid_inst(cpu.pc);
  }

  // ebreak check
  if(ifbreak){
    printf("\nebreak!\n");
    // printf("ebreak: pc = 0x%08x inst = 0x%08x\n",cpu.pc,dut->inst);
    // set_npc_state(NPC_STOP, cpu.pc, 0);ifbreak = false;
    NPCTRAP(cpu.pc, 0);
  }

  #if (defined CONFIG_TRACE) || (defined CONFIG_TRACE)
  s.snpc = s.pc + 4;
  #endif
  // printf("inst = [0x%08x]----pc = [0x%08x]\n",*(uint32_t*)inst,cpu.pc);

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

  // 1.判断函数调用/函数返回
  uint32_t m = s.isa.inst.val;
  // printf("m = %08x\n",m);
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
    char name[20] = {0};

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
        if (if_return && (sym.st_value <= s.pc && sym.st_value + sym.st_size >= s.pc) && sym.st_info == 18){
          // printf("sym.st_value = 0x%08x sym.st_size = %d \n",sym.st_value,sym.st_size);
          break;
        }
        // 3.2.2 函数调用 是跳转到一个新函数的头部
        else if (!if_return && sym.st_value == s.dnpc && sym.st_info == 18){
          // printf("sym.st_value = 0x%08x s.dnpc = 0x%08x \n",sym.st_value,s.dnpc);
          break;
        }
      }
      if(n >= 0) break;
      if (indx == fileNum - 1){
          // if_same = true;
          Assert(0, "Fail in searching!");
        }
    }
    
    
      // 取出函数名称
      strncpy(name, elf_header[indx].strtab + sym.st_name, 19);
    // 4.调用的函数放入一个数据结构，返回函数放入一个数据结构
    static int index = 1;

    if (if_return){
      // 函数返回，将函数名所在链表节点抽出
        // Assert(name, "name NULL!");
        if(strcmp(name,"putch") != 0) printf("index %d-> 0x%08x: \033[106m ret [%s] \033[m\n", index, cpu.pc, name);
        index++;
    }
    else{
      // 函数调用，将函数名放入链表
      if(strcmp(name,"putch") != 0) printf("index %d-> 0x%08x: \033[102m call[%s @0x%08x] \033[m\n", index, cpu.pc, name, s.dnpc);
      #ifdef CONFIG_FTRACE_PASS
      if(strcmp(name,"putch") != 0) 
        for(int i = 10; i < 15; i++){
          printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,regs[i],gpr(i));
        }
      #endif
      index++;
    }
  }

#endif

}

/* stimulate the way CPU works ,get commands constantly */
static void execute(uint64_t n) {
  for (; n > 0; n--)
  {
    exec_once();
    g_nr_guest_inst++;  //记录客户指令的计时器
    //由于rtl对reg的更改是在下一个时钟周期上升沿，而nemu对reg的更改是即时的
    //所以这里要整个往后延迟一个周期
    #ifdef CONFIG_DIFFTEST
    diff.pc = s.pc;
    diff.dnpc = s.dnpc;
    #endif
    // if(cpu.pc != 0x80000000) {
    // printf("trace and diff\n");
    trace_and_difftest();
// }
  
    //当npc_state.state被设置为NPC_STOP时，npc停止执行指令
    if (npc_state.state != NPC_RUNNING)
      break;
    IFDEF(CONFIG_DEVICE, device_update());
  }
}

uint64_t clk_cycle = 0;

void clk_cycle_plus(){
  clk_cycle += 2;
}

// static void statistic_sim() {
//   Log("simulation frequency = " NUMBERIC_FMT " cycle/s", clk_cycle * 1000000 / g_timer); 
// }

static void statistic()
{
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  Log("host time spent = " NUMBERIC_FMT " us", g_timer);
  Log("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0){
    Log("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
    Log("simulation frequency = " NUMBERIC_FMT " cycle/s", clk_cycle * 1000000 / g_timer);} 
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
				dut->final();
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