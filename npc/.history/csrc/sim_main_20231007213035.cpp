/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/

#include "npc.h"
#include "common.h"
#include "ifetch.h"
#include "sim.h"

void engine_start();
int is_exit_status_bad();


#define MAX_SIM_TIME 100
vluint64_t sim_time = 0;

TOP_NAME dut;

// void reset_pc(){
// 	dut.clk = 0; 
// 	dut.eval();
// 	dut.rst = 1;
// 	dut.eval();

// 	m_trace->dump(sim_time);
// 	sim_time++;

// 	dut.clk = 1;
// 	dut.eval(); 
// 	dut.rst = 0;
// 	dut.eval();

// 	dut.inst = inst_fetch((vaddr_t)dut.pc, 4);
// 	dut.eval();
// 	m_trace->dump(sim_time);
// 	sim_time++;
// }


int main(int argc, char** argv, char** env) {
	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
	VerilatedVcdC *m_trace = new VerilatedVcdC;

	init_npc(argc, argv);

	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
	
	//reset_pc();

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

	dut.inst = inst_fetch((vaddr_t)dut.pc, 4);
	dut.eval();
	m_trace->dump(sim_time);
	sim_time++;

	/* Start engine. */
	engine_start();

	dut.final();
	m_trace->close();	//关闭波形跟踪文件

	//exit(EXIT_SUCCESS);
	return is_exit_status_bad();
}


// int main(int argc, char** argv, char** env) {
// 	Verilated::traceEverOn(true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
// 	VerilatedVcdC *m_trace = new VerilatedVcdC;

// 	parseArgs(argc, argv);

// 	init_mem_npc();  //初始化内存

// 	load_img();

// 	dut.trace(m_trace, 5);               
// 	m_trace->open("waveform.vcd");
	
// 	dut.clk = 0; 
// 	dut.eval();
// 	dut.rst = 1;
// 	dut.eval();

// 	m_trace->dump(sim_time);
// 	sim_time++;

// 	dut.clk = 1;
// 	dut.eval(); 
// 	dut.rst = 0;
// 	dut.eval();

// 	int n = 0;
// 	dut.inst = pmem_read_npc(dut.pc);
	

// 	dut.eval();
// 	m_trace->dump(sim_time);
// 	sim_time++;

// 	while (sim_time < MAX_SIM_TIME) {		
// 		dut.clk ^= 1;
// 		dut.eval();
// 		if(dut.clk == 1) {
// 			dut.inst = pmem_read_npc(dut.pc);
// 		}
// 		dut.eval();
// 		m_trace->dump(sim_time);
// 		sim_time++;

// 		if(ifbreak) {
// 			printf("\nebreak!\n");
// 			break;
// 		}
// 	}
// 	dut.final();
// 	m_trace->close();	//关闭波形跟踪文件
// 	exit(EXIT_SUCCESS);
// }


