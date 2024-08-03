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

#include "cpu.h"
#include <locale.h>
#include "sdb.h"
#include <elf.h>

#include "isa.h"
#include "sim.h"

#include "npc.h"
#include "common.h"
#include "ifetch.h"

extern TOP_NAME dut;


bool ifbreak = false;


/* The assembly code of instructions executed is only output to the screen
 * when the number of instructions executed is less than this value.
 * This is useful when you use the `si' command.
 * You can modify this value as you want.
 */
#define MAX_INST_TO_PRINT 10

uint64_t g_nr_guest_inst = 0;
static uint64_t g_timer = 0; // unit: us
static bool g_print_step = false;
extern vluint64_t sim_time;
extern VerilatedVcdC *m_trace;


// typedef struct watchpoint {
//   int NO;
//   int times;
//   uint32_t data;
//   char target[100];
//   struct watchpoint *next;
//   struct watchpoint *past;

//   /* TODO: Add more members if necessary */

// } WP;

// WP* get_head();

void ifebreak_func(int key){
	printf("while key = %d\n",key);
	if(key == 9) {ifbreak = true; } 
}

/* let CPU conduct current command and renew PC */
static void exec_once()
{
    printf("begin ifetch! clk = %d\n",dut.clk);
    dut.clk ^= 1;
    dut.eval();
    if(dut.clk == 1) {
      printf("helo!\n");
        printf("pc = 0x%08x   ",dut.pc);
        dut.inst = inst_fetch((vaddr_t)dut.pc, 4);
        printf("pc = 0x%08x nst = %032x\n",dut.pc,dut.inst);
    }
    dut.eval();
    m_trace->dump(sim_time);
    sim_time++;
}

/* stimulate the way CPU works ,get commands constantly */
static void execute(uint64_t n)
{
  for (; n > 0; n--)
  {
    printf("------------------------------------------\n");
    if(dut.clk == 1){
      printf("clk == 0\n");
      exec_once();
      n ++;
      printf("------------------------------------------\n");
      continue;
    } 
    else{
      exec_once();
    }
    printf("\nCPU runing [%d]\n",n);
    printf("------------------------------------------\n");
    g_nr_guest_inst++;  //记录客户指令的计时器
    if(ifbreak) {
        printf("\nebreak!\n");
        break;
    }
  }
}

static void statistic()
{
  IFNDEF(CONFIG_TARGET_AM, setlocale(LC_NUMERIC, ""));
#define NUMBERIC_FMT MUXDEF(CONFIG_TARGET_AM, "%", "%'") PRIu64
  printf("host time spent = " NUMBERIC_FMT " us", g_timer);
  printf("total guest instructions = " NUMBERIC_FMT, g_nr_guest_inst);
  if (g_timer > 0)
    printf("simulation frequency = " NUMBERIC_FMT " inst/s", g_nr_guest_inst * 1000000 / g_timer);
  else
    printf("Finish running in less than 1 us and can not calculate the simulation frequency");
}

// void assert_fail_msg()
// {
//   iringbuf_display();
//   isa_reg_display();
//   statistic();
// }

/* Simulate how the CPU works. */
void cpu_exec(uint64_t n)
{
  uint64_t timer_start = get_time();

  execute(n);

  uint64_t timer_end = get_time();
  g_timer += timer_end - timer_start;

  if(ifbreak) {
    statistic();

  }
}
