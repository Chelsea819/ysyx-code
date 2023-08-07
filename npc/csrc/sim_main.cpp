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
#include <iostream>
#include <iomanip> // 用于设置输出格式
#include <cstdint> // 包含 uint32_t 类型

// *pmem = 0b 000000000001 00000 000 000010 010011;
// *(pmem + 1) = 0b 00000000010 00000 000 000010 0010011;
// *(pmem + 2) = 0b 00000000011 00000 000 000011 0010011;
// *(pmem + 3) = 0b 00000000100 00000 000 000100 0010011;


static  TOP_NAME dut;
//void nvboard_bind_all_pins(Vtop* top);
static uint32_t *pmem = NULL;

static int init_mem(){
	pmem = (uint32_t *)malloc(sizeof(uint32_t)*10);
	assert(pmem);
	*pmem = 0b00000000000100000000000010010011;
	*(pmem + 1) = 0b00000000010000000000000100010011;
	*(pmem + 2) = 0b00000000011000000000000110010011;
	*(pmem + 3) = 0b00000000100000000000001000010011;

	return 0;
}

static inline uint32_t host_read(void *addr) {
    return *(uint32_t *)addr;
}

uint32_t* guest_to_host(uint32_t paddr) { return pmem + (paddr / 32); }

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
	int flag = 16;
	printf("before while\n");
	while (sim_time < MAX_SIM_TIME) {
//	while(1){
		dut.clk ^= 1; 
		printf("0x%032x\n",dut.pc);
		printf("before pmem_read\n");
		dut.inst = pmem_read(dut.pc);
		printf("dut.inst = 0x%032x\n",dut.inst);
		printf("after pmem_read\n");
		dut.eval();
		m_trace->dump(sim_time);
		sim_time++;
	//	nvboard_update();
	//	usleep(1);
		dut.rst = 0;
		printf("flag = %d\n",flag);
		if(!(flag --)) break;
	}
	free(pmem);
	pmem = NULL;
	m_trace->close();
	//delete dut;
	exit(EXIT_SUCCESS);
}





