/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/

#include "svdpi.h"
#include "Vysyx_22041211_top__Dpi.h"
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vysyx_22041211_top.h"
#include "Vysyx_22041211_top___024root.h"


#define MAX_SIM_TIME 100
#define PG_ALIGN __attribute((aligned(4096)))
#define CONFIG_MSIZE 0x8000000

vluint64_t sim_time = 0;
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
bool ifbreak = false;

TOP_NAME dut;

static const uint32_t img [] = {
  0x1c00000c,  // pcaddu12i $t0,0
  0x29804180,  // st.w $zero,$t0,16
  0x28804184,  // ld.w $a0,$t0,16
  0x002a0000,  // break 0 (used as nemu_trap)
  0xdeadbeef,  // some data
};


void ifebreak_func(int inst){
	//printf("while key = %d\n",key);
	if(inst == 1048691) {ifbreak = true; } 
}

int main(int argc, char** argv, char** env) {
	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
	VerilatedVcdC *m_trace = new VerilatedVcdC;

	memcpy(guest_to_host(RESET_VECTOR), img, sizeof(img)); //初始化内存

	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
	
	dut.clk = 0; 
	dut.eval();
	dut.rst = 1;
	dut.eval();

	m_trace->dump(sim_time);
	sim_time++;

	dut.clk = 1;
	dut.eval(); 
	dut.rst = 0;
	dut.eval();
	dut.inst = pmem_read_npc(dut.pc);
	dut.eval();

	m_trace->dump(sim_time);
	sim_time++;

	while (sim_time < MAX_SIM_TIME) {		
		dut.clk ^= 1;
		dut.eval();
		//上升沿取指令
		if(dut.clk == 1) {
			dut.inst = pmem_read_npc(dut.pc);
			dut.eval();
		}
		m_trace->dump(sim_time);
		sim_time++;

		if(ifbreak) {
			printf("\nebreak!\n");
			break;
		}
	}

	dut.final();
	m_trace->close();	//关闭波形跟踪文件
	exit(EXIT_SUCCESS);
}



