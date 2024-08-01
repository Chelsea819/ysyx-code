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
#include <memory/paddr.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x1c00000c,  // pcaddu12i $t0,0
  0x29804180,  // st.w $zero,$t0,16
  0x28804184,  // ld.w $a0,$t0,16
  0x002a0000,  // break 0 (used as nemu_trap)
  0xdeadbeef,  // some data
};

static void restart() {
  /* Set the initial program counter. */
  //RESET_VECTOR是一个固定的重置地址
  cpu.pc = RESET_VECTOR;

  /* The zero register is always 0. */
  //将通用寄存器0号gpr[0]设置为0
  cpu.gpr[0] = 0;
}

//用来初始化整个虚拟计算机系统
void init_isa() {
  /* Load built-in image. */
  /*使用memcpy()将 img 
  复制到从RESET_VECTOR开始的内存区域*/
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

  /* Initialize this virtual computer system. */
  //将程序计数器pc设为重置地址RESET_VECTOR
  restart();
}
