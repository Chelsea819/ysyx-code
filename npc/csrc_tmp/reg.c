#include <isa.h>
#include "reg.h"

uint32_t convert_ten(char *args);
extern TOP_NAME dut;
const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

void isa_reg_display() {
  for(int i = 0; i < 32; i++){
    printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,regs[i],gpr(i));
  }
  printf("\033[102m PC: \033[0m \t0x%08x\n",dut.pc);
  return;
}

word_t isa_reg_str2val(char *s, bool *success) {
  //printf("args = _%s_\n",s);
  if(strcmp("pc",s) == 0){
      *success = true;
      //printf("strcmp(pc,s) == 0\n");
      free(s);
      return dut.pc;
  }  
  for(int i = 0; i < 32; i++){
    if(strcmp(regs[i],s) == 0){
      *success = true;
      free(s);
      return gpr(i);
    }
  } 
  return 0;
}