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
#include <paddr.h>

// this is not consistent with uint8_t
// but it is ok since we do not access the array directly
static const uint32_t img [] = {
  0x00010537,  // lui	a0,0x10
  0x01050513,  // auipc t0,0
  0x00010537,  // addi    x[1] = 0 + 1
  0x01050513,  // auipc t0,0
  0x00100073,  // ebreak (used ass npc_trap)
  0x00100093,  // addi    x[1] = 0 + 1
};

// static void restart() {
//   /* Set the initial program counter. */
//   cpu.pc = RESET_VECTOR;

//   /* The zero register is always 0. */
//   cpu.gpr[0] = 0;
// }

// void init_isa() {
//   /* Load built-in image. */
//   memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));

//   /* Initialize this virtual computer system. */
//   restart();
// }
void init_isa() {
  /* Load built-in image. */
  memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img));
}