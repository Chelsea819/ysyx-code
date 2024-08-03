#include <isa.h>
#include <cpu/difftest.h>
#include "reg.h"

// #include "sim.h"

extern CPU_state cpu;
extern TOP_NAME dut;

// 把通用寄存器和PC与从DUT中读出的寄存器的值进行比较.
bool isa_difftest_checkregs(CPU_state *ref_r, vaddr_t pc) {
  if(cpu.pc!= ref_r->pc) return false;
  for(int i = 0; i < 32; i++){
    if(ref_r->gpr[i] != R(i)) return false;
  } 
  return true;
}

void isa_difftest_attach() {
}