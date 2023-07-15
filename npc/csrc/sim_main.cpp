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

static  TOP_NAME *dut;
void nvboard_bind_all_pins(Vtop* top);


int main(int argc, char** argv, char** env) {

//	Vtop *dut = new Vtop;


	nvboard_bind_all_pins(dut);
	nvboard_init();

	Verilated::traceEverOn(true);
	VerilatedVcdC *m_trace = new VerilatedVcdC;  
	dut->trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
	while (sim_time < MAX_SIM_TIME) {
		int a = rand() & 1;
		int b = rand() & 1;
		//	dut->clk ^= 1; 
		dut->a = a;
		dut->b = b;
		dut->eval();
		printf("a = %d, b = %d, f = %d\n", a, b, dut->f);
		m_trace->dump(sim_time);
		sim_time++;
		assert(dut->f == (a ^ b));
		nvboard_update();
	}
	m_trace->close();
	delete dut;
	exit(EXIT_SUCCESS);
}





