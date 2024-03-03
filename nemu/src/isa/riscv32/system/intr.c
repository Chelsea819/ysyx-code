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

#include <isa.h>
// const char *regs2[] = {
//   "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
//   "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
//   "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
//   "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
// };

word_t isa_raise_intr(word_t NO, vaddr_t epc) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * Then return the address of the interrupt/exception vector.
   */
  // SR[mepc] <- PC
  // SR[mcause] <- 一个描述失败原因的号码
  // PC <- SR[mtvec]
  // for(int i = 0; i < 32; i++){
  //   printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,regs2[i],cpu.gpr[i]);
  // }
  // printf("\033[102m PC: \033[0m \t0x%08x\n",cpu.pc);

  cpu.csr[CSR_MEPC] = epc;
  cpu.csr[CSR_MCAUSE] = 8;
  cpu.pc = cpu.csr[CSR_MTVEC];
  cpu.csr[CSR_MSTATUS] = 0x1800;
  // printf("cpu.pc = 0x%08x\n",cpu.pc);
  // printf("cpu.csr[CSR_MTVEC] = 0x%08x\n",cpu.csr[CSR_MTVEC]);
  // printf("cpu.csr[CSR_MEPC]: 0x%08x\n",cpu.csr[CSR_MEPC]);
  // printf("cpu.csr[CSR_MCAUSE]: 0x%08x\n",cpu.csr[CSR_MCAUSE]);
  // printf("cpu.csr[773] = 0x%08x\n",csr(773));
  #ifdef CONFIG_ETRACE
  Log("[ETRACE]: CSR[mepc] = 0x%08x CSR[mtvec]: 0x%08x CSR[mcause]: 0x%08x\n",epc,cpu.csr[CSR_MTVEC],NO);
  #endif
  return cpu.csr[CSR_MTVEC];
  // return 0;
}

word_t isa_query_intr() {
  return INTR_EMPTY;
}
