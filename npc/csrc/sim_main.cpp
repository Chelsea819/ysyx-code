/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/
#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <stdio.h>
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vtop.h"
#include "Vtop___024root.h"
#include <nvboard.h>

#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;

static  TOP_NAME dut;
void nvboard_bind_all_pins(Vtop* top);


int main(int argc, char** argv, char** env) {

	nvboard_bind_all_pins(&dut);
	nvboard_init();

	Verilated::traceEverOn(true);
	VerilatedVcdC *m_trace = new VerilatedVcdC;  
	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
//	while (sim_time < MAX_SIM_TIME) {
	while(1){
		int a1 = rand() & 1;
		int b1 = rand() & 1;
		//	dut->clk ^= 1; 
		dut.a = a1;
		dut.b = b1;
		dut.eval();
		printf("a = %d, b = %d", a1, b1);
		m_trace->dump(sim_time);
		sim_time++;
		assert(dut.f == (a1 ^ b1));
		nvboard_update();
		sleep(7);
	}
	m_trace->close();
	//delete dut;
	//
	exit(EXIT_SUCCESS);
}





