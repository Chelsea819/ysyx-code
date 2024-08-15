/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NPC is licensed under Mulan PSL v2.
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
#include "reg.h"

uint32_t convert_ten(char *args);

#ifdef CONFIG_RVE
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5"
};
#else
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};
#endif

void isa_reg_display() {
  if(cpu.pc == 0x80000000) return;
  for(int i = 0; i < REG_NUM; i++){
    printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,reg_name(i),R(i));
  }
  printf("\033[103m %d: \033[0m \t  \033[104m %s: \033[0m \t0x%08x\n",0,"macuse",cpu.mcause);
  printf("\033[103m %d: \033[0m \t  \033[104m %s: \033[0m \t0x%08x\n",1,"mepc",cpu.mepc);
  printf("\033[103m %d: \033[0m \t  \033[104m %s: \033[0m \t0x%08x\n",2,"mstatus",cpu.mstatus);
  printf("\033[103m %d: \033[0m \t  \033[104m %s: \033[0m \t0x%08x\n",3,"mtvec",cpu.mtvec);

  printf("\033[102m PC: \033[0m \t0x%08x\n",cpu.pc);
  return;
}

word_t isa_reg_str2val(char *s, bool *success) {
  // printf("args = _%s_\n",s);
  if(strcmp("pc",s) == 0){
      *success = true;
      //printf("strcmp(pc,s) == 0\n");
      free(s);
      return dut->pc;
  }  
  for(int i = 0; i < 32; i++){
    if(strcmp(reg_name(i),s) == 0){
      *success = true;
      free(s);
      return R(i);
    }
  } 
  return 0;
}
