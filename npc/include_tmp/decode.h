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

#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include "isa.h"

typedef struct Decode {
  vaddr_t pc;
  vaddr_t snpc; // static next pc 静态指令是指程序代码中的指令
  vaddr_t dnpc; // dynamic next pc 动态指令是指程序运行过程中的指令
  ISADecodeInfo isa; //用于存放ISA相关的译码信息
  IFDEF(CONFIG_ITRACE, char logbuf[128]);
} Decode;   //译码信息结构体Decode


#endif
