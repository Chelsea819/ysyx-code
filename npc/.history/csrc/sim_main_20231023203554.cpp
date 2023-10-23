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
vluint64_t sim_time = 0;
static uint32_t pmem[20] = {0};
bool ifbreak = false;

TOP_NAME dut;

void init_mem_npc(){
	//80000000-80000004-80000008-8000000c-80000010-80000014-80000024-80000028-8000002c-80000018-8000001c-80000020-80000030
	pmem[0] = 0b00000000000100000000000010010011; //addi    x[1] = 0 + 1     	80000000	1048723	100093
	pmem[1] = 0b00000000010000000000000100010011; //2   x[1] = 0 + 2 			80000004	4194579	400113	
	pmem[2] = 0b00000000000000000001010000010111; //auipc						80000008	5143
	pmem[3] = 0b00000000000000000010010000010111; //auipc						8000000c	9239
	pmem[4] = 0b00000000000000000110000110110111; //lui							80000010	25015
	pmem[5] = 0b00000000100000000000000011101111; //jal   x[1] = pc + 4 pc += 8 80000014	8388847
	pmem[6] = 0b00000000000000001110001110110111; //lui							80000018	58295
	pmem[7] = 0b00000000000000011110011110110111; //lui							8000001c	124855
	pmem[8] = 0b00000000100000000000000011101111; //jal   x[1] = pc + 4 pc += 8 80000020	8388847
	pmem[9] = 0b00000000111000000000001110010011; //addi x[3] = 0 + 6 			80000024	14680979
	pmem[10] = 0b00000001000000000000010000010011; //addi x[4] = 0 + 7			80000028	16778259
	pmem[11] = 0b00000000000000001000000111100111; //jalr 					    8000002c   	41447		x[3] = pc + 4 pc = x[1] + imm = 80000018 + 0
	pmem[12] = 0b00000000000100000000000001110011; //ebreak						80000030
	return ;
}


void ifebreak_func(int inst){
	//printf("while key = %d\n",key);
	if(inst == 1048691) {ifbreak = true; } 
}

int main(int argc, char** argv, char** env) {
	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
	VerilatedVcdC *m_trace = new VerilatedVcdC;

	init_mem_npc();  //初始化内存

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



