#include <dlfcn.h>

#include <isa.h>
#include <cpu.h>
#include "paddr.h"
#include <utils.h>
#include <difftest-def.h>
#include <debug.h>
#include "reg.h"
#include "config.h"

extern TOP_NAME dut;
#ifdef CONFIG_WAVE
extern VerilatedVcdC *m_trace;
#endif
extern CPU_state cpu;

#ifdef CONFIG_DIFFTEST
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc);


void (*ref_difftest_memcpy)(paddr_t addr, void *buf, size_t n, bool direction) = NULL;
void (*ref_difftest_regcpy)(void *duti, bool direction) = NULL;
void (*ref_difftest_exec)(uint64_t n) = NULL;
void (*ref_difftest_raise_intr)(uint64_t NO) = NULL;

static vaddr_t is_skip_ref_pc = 0;
static bool is_skip_ref = false;
static int skip_dut_nr_inst = 0;

// this is used to let ref skip instructions which
// can not produce consistent behavior with NEMU
void difftest_skip_ref() {
  is_skip_ref = true;
  is_skip_ref_pc = dut.pc;
  printf("is_skip_ref_pc = 0x%08x cpu.pc = 0x%08x dut.pc = 0x%08x\n",is_skip_ref_pc,cpu.pc,dut.pc);
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
//difftest_skip_dut还需要修改，因为nemu和npc错位的问题

const char *regs1[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

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
  ref_difftest_memcpy(RESET_VECTOR, guest_to_host_npc(RESET_VECTOR), img_size, DIFFTEST_TO_REF);
  
  //将DUT的寄存器状态拷贝到REF中
  ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
}

static void checkregs(CPU_state *ref, vaddr_t pc) {
  if (!isa_difftest_checkregs(ref, pc)) {
    npc_state.state = NPC_ABORT;
    npc_state.halt_pc = pc;
    for(int i = 0; i < 32; i++){
        printf("\033[103m %d: \033[0m \t0x%08x  \033[104m %s: \033[0m \t0x%08x\n",i,ref->gpr[i],regs1[i],R(i));
    }
    printf("\033[103m ref->pc: \033[0m \t0x%08x  \033[104m cpu.pc: \033[0m \t0x%08x\n",ref->pc,cpu.pc);
    dut.final();
  #ifdef CONFIG_WAVE
    m_trace->close();
  #endif
    Assert(0,"Catch difference!\n");
  }
}

//进行逐条指令执行后的状态对比
void difftest_step(vaddr_t pc, vaddr_t npc) {
  CPU_state ref_r;
  printf("pc = 0x%08x npc = 0x%08x\n",pc,npc);
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
  if (is_skip_ref && is_skip_ref_pc == pc) {
    Log("pc = 0x%08x npc = 0x%08x\n",pc,npc);
    // to skip the checking of an instruction, just copy the reg state to reference design
    ref_difftest_regcpy(&cpu, DIFFTEST_TO_REF);
    is_skip_ref = false;
    is_skip_ref_pc = 0;
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
