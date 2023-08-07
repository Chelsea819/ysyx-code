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
#include "Vysyx_22041211_top.h"
#include "Vysyx_22041211_top___024root.h"
//#include <nvboard.h>

#define MAX_SIM_TIME 20
vluint64_t sim_time = 0;
//#define CONFIG_MSIZE 0X80000000
#define CONFIG_MBASE 0X80000000


static  TOP_NAME dut;
//void nvboard_bind_all_pins(Vtop* top);
static uint32_t *pmem = NULL;

static int init_mem(){
	pmem = (uint32_t *)malloc(sizeof(uint32_t)*10);
	assert(pmem);
	*pmem = 0b10000000000000000000000010010011;
	*(pmem + 1) = 0b01000000000000000000000100010011;
	*(pmem + 2) = 0b11000000000000000000000110010011;
	*(pmem + 3) = 0b00100000000000000000001000010011;

	return 0;
}

static inline uint32_t host_read(void *addr) {
    return *(uint32_t *)addr;
}

uint32_t* guest_to_host(uint32_t paddr) { return pmem + paddr - CONFIG_MBASE; }

static uint32_t pmem_read(uint32_t addr) {
  uint32_t ret = host_read(guest_to_host(addr));
  return ret;
}


int main(int argc, char** argv, char** env) {

	//nvboard_bind_all_pins(&dut);
	//nvboard_init();

	Verilated::traceEverOn(true);
	VerilatedVcdC *m_trace = new VerilatedVcdC;  
	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
	init_mem();
	dut.rst = 1;
	int flag = 2;
	while (sim_time < MAX_SIM_TIME) {
//	while(1){
		dut.clk ^= 1; 
		dut.inst = pmem_read(dut.pc);
		dut.eval();
		m_trace->dump(sim_time);
		sim_time++;
	//	nvboard_update();
	//	usleep(1);
		dut.rst = 0;
		if(flag --) break;
	}
	free(pmem);
	pmem = NULL;
	m_trace->close();
	//delete dut;
	exit(EXIT_SUCCESS);
}





