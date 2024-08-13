#include <dlfcn.h>

#include <isa.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <utils.h>
#include <difftest-def.h>
#include <debug.h>
#include "reg.h"
#include "config.h"

#ifdef CONFIG_WAVE
extern VerilatedVcdC *m_trace;
#endif
extern CPU_state cpu;

#ifdef CONFIG_DIFFTEST
int isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc);


void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *duti, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

struct is_skip_ref{
  bool is_skip_ref_bool;
  vaddr_t is_skip_ref_pc;
  struct is_skip_ref *next;
  struct is_skip_ref *past;
};
#define NR_SKIP 2
static struct is_skip_ref *head = NULL;
static struct is_skip_ref *tail = NULL;
static struct is_skip_ref skip_pool[2];

static int skip_dut_nr_inst = 0;

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref() {
  tail->is_skip_ref_bool = true;
  tail->is_skip_ref_pc = dut->pc;
  // printf("tail->is_kskip_ref_pc = 0x%08x cpu.pc = 0x%08x dut->pc = 0x%08x\n",tail->is_skip_ref_pc,cpu.pc,dut->pc);

  tail = tail->next;
  // is_skip_ref = true;
  // is_skip_ref_pc = dut->pc;
  // If such an instruction is one of the instruction packing in QEMU
  // (see below), we end the process of catching up with QEMU's pc to
  // keep the consistent behavior in our best.
  // Note that this is still not perfect: if the packed instructions
  // already write some memory, and the incoming instruction in NEMU
  // will load that memory, we will encounter false negative. But such
  // situation is infrequent.
  skip_dut_nr_inst = 0;
}

// this is used to deal with instruction packing in QEMU.
// Sometimes letting QEMU step once will execute multiple instructions.
// We should skip checking until NEMU's pc catches up with QEMU's pc.
// The semantic is
//   Let REF run `nr_ref` instructions first.
//   We expect that DUT will catch up with REF within `nr_dut` instructions.
void difftest_skip_dut(int nr_ref, int nr_dut) {
  skip_dut_nr_inst += nr_dut;

  while (nr_ref -- > 0) {
    ref_difftest_exec(1);
  }
}


void init_skip_pool(){
  for (int i = 0; i < NR_SKIP; i ++) {
    skip_pool[i].is_skip_ref_bool = false;
    skip_pool[i].is_skip_ref_pc = 0;
    skip_pool[i].next = (i == NR_SKIP - 1 ? skip_pool : &skip_pool[i + 1]);
    skip_pool[i].past = (i == 0 ? &skip_pool[NR_SKIP - 1] : &skip_pool[i - 1]);
  }
  head = skip_pool;
  tail = skip_pool;
}

void init_difftest(char *ref_so_file, long img_size, int port) {
  assert(ref_so_file != NULL);

  //打开传入的动态库文件
  void *handle;
  handle = dlopen(ref_so_file, RTLD_LAZY);
  assert(handle);

  //通过动态链接对动态库中的上述API符号进行符号解析和重定位, 返回它们的地址
  ref_difftest_memcpy = (void (*)(paddr_t addr, void *buf, size_t n, bool direction))dlsym(handle, "difftest_memcpy");
  assert(ref_difftest_memcpy);

  ref_difftest_regcpy = (void (*)(void *duti, bool direction))dlsym(handle, "difftest_regcpy");
  assert(ref_difftest_regcpy);

  ref_difftest_exec = (void (*)(uint64_t n))dlsym(handle, "difftest_exec");
  assert(ref_difftest_exec);

  ref_difftest_raise_intr = (void (*)(uint64_t NO))dlsym(handle, "difftest_raise_intr");
  assert(ref_difftest_raise_intr);

  void (*ref_difftest_init)(int) = (void (*)(int))dlsym(handle, "difftest_init");
  assert(ref_difftest_init);

  Log("Differential testing: %s", ANSI_FMT("ON", ANSI_FG_GREEN));
  Log("The result of every instruction will be compared with %s. "
      "This will help you a lot for debugging, but also significantly reduce the performance. "
      "If it is not necessary, you can turn it off in menuconfig.", ref_so_file);
  
  //对REF的DIffTest功能进行初始化
  ref_difftest_init(port);
  
  //将DUT的guest memory拷贝到REF中
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  //将DUT的寄存器状态拷贝到REF中
  ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);

  init_skip_pool();

}

static void checkregs(CPU_state *ref, vaddr_t pc) {
  int flag = 0;
  if ((flag = isa_difftest_checkregs(ref, pc)) != 0) {
    npc_state.state = NPC_ABORT;
    npc_state.halt_pc = pc;
    for(int i = 0; i < RISCV_GPR_NUM; i++){
      if (flag == i + 1) 
        printf("\033[105m %d:  \t0x%08x\033[0m  \033[106m %s:  \t0x%08x\033[0m\n",i,ref->gpr[i],regs[i],cpu.gpr[i]);
      else
        printf("\033[103m %d: \033[0m \t0x%08x  \033[104m %s: \033[0m \t0x%08x\n",i,ref->gpr[i],regs[i],cpu.gpr[i]);
    }
    if (flag == 33) 
      printf("\033[105m ref->pc: \033[0m \t0x%08x  \033[106m cpu.pc: \033[0m \t0x%08x\n",ref->pc,cpu.pc);
    else
      printf("\033[103m ref->pc: \033[0m \t0x%08x  \033[104m cpu.pc: \033[0m \t0x%08x\n",ref->pc,cpu.pc);
    dut->final();
  #ifdef CONFIG_WAVE
    m_trace->close();
  #endif
    Assert(0,"Catch difference!\n");
  }
}

//进行逐条指令执行后的状态对比
void difftest_step(vaddr_t pc, vaddr_t npc) {
  CPU_state ref_r;
  // printf("pc = 0x%08x npc = 0x%08x\n",pc,npc);
  if (skip_dut_nr_inst > 0) {
    ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
    if (ref_r.pc == npc) {
      skip_dut_nr_inst = 0;
      checkregs(&ref_r, npc);
      return;
    }
    skip_dut_nr_inst --;
    if (skip_dut_nr_inst == 0)
      panic("can not catch up with ref.pc = " FMT_WORD " at pc = " FMT_WORD, ref_r.pc, pc);
    return;
  }

  //该指令的执行结果以NEMU的状态为准
  if (head->is_skip_ref_bool && pc == head->is_skip_ref_pc) {
    // Log("pc = 0x%08x npc = 0x%08x\n",pc,npc);
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    head->is_skip_ref_bool = false;
    head->is_skip_ref_pc = 0;
    head = head->next;
    return;
  }
// ref 0x8000 0x8004 0x8008 0x800c
// dut 0x8000 0x8000 0x8004 0x8008

// ref 0x8004 0x8008 0x800c 0x800f
// dut 0x8004 0x8008 0x800c 0x800f
  ref_difftest_exec(1);
  ref_difftest_regcpy(&ref_r, DIFFTEST_TO_DUT);
  checkregs(&ref_r, pc);
}
#else
void init_difftest(char *ref_so_file, long img_size, int port) { }
#endif
