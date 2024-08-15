#include <isa.h>
#include <cpu/difftest.h>
#include "reg.h"

// #include "sim.h"

extern CPU_state cpu;
extern TOP_NAME *dut; extern VerilatedVcdC *m_trace;

// 把通用寄存器和PC与从DUT中读出的寄存器的值进行比较.
int isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if(cpu.pc!= ref_r->pc) return 33;
  for(int i = 0; i < RISCV_GPR_NUM; i++){
    if(ref_r->gpr[i] != cpu.gpr[i]) return (i + 1);
  } 
  if (ref_r->mcause != cpu.mcause) {
    return 34;
  } else if (ref_r->mstatus != cpu.mstatus) {
    return 35;
  } else if (ref_r->mepc != cpu.mepc) {
    return 36;
  } else if (ref_r->mtvec != cpu.mtvec) {
    return 37;
  }
  return 0;
}

void isa_difftest_attach() {
}