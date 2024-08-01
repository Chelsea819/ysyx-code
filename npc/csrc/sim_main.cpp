#ifndef __NPC_CONFIG_H__
#define __NPC_CONFIG_H__

// #define CONFIG_MTRACE 1
// #define CONFIG_FTRACE 1
// #define CONFIG_DIFFTEST 1
// #define CONFIG_ITRACE_COND 1
// #define CONFIG_ITRACE 1
// #define CONFIG_TRACE 1
#define CONFIG_DEVICE 1
// #define CONFIG_DTRACE 1
#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000
#define CONFIG_PC_RESET_OFFSET 0x0
#define CONFIG_RT_CHECK 1
#define CONFIG_ISA_riscv 1
#define CONFIG_TARGET_NATIVE_ELF 1

// #define CONFIG_TRACE_START 0
// #define CONFIG_TRACE_END 10000

// #define CONFIG_WAVE 1


#define ANTO_Q 1

#endif

#include "svdpi.h"
#include "Vysyx_22041211_top__Dpi.h"
#include <verilated.h>
#ifdef CONFIG_WAVE
#include <verilated_vcd_c.h>
#endif
#include "Vysyx_22041211_top.h"
#include "Vysyx_22041211_top___024root.h"
TOP_NAME dut;
void init_npc(int argc,char *argv[]);
void engine_start();

int main(int argc, char** argv, char** env) {
#ifdef CONFIG_WAVE
	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
#else	
	Verilated::traceEverOn(false); 
#endif
	init_npc(argc, argv);

#ifdef CONFIG_WAVE
	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
#endif	

	dut.clk = 0; 
	dut.eval();
	dut.rst = 1;
	dut.eval();
#ifdef CONFIG_WAVE
  m_trace->dump(sim_time);
	sim_time++;
#endif	
  dut.clk = 1;
  dut.eval();
  dut.rst = 0;
	dut.eval();
#ifdef CONFIG_WAVE
  m_trace->dump(sim_time);
	sim_time++;
#endif
  /* Start engine. */
	engine_start();

	dut.final();
#ifdef CONFIG_WAVE 
	m_trace->close();	//关闭波形跟踪文件
#endif
	exit(EXIT_SUCCESS);
}