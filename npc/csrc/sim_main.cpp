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
#include "svdpi.h"
#include "Vysyx_22041211_top__Dpi.h"
#include <verilated.h>
#include <verilated_vcd_c.h>
#include "Vysyx_22041211_top.h"
#include "Vysyx_22041211_top___024root.h"
//#include <nvboard.h>

#define MAX_SIM_TIME 100
vluint64_t sim_time = 0;
//#define CONFIG_MSIZE 0X80000000
#define CONFIG_MBASE 0X80000000



static  TOP_NAME dut;
//void nvboard_bind_all_pins(Vtop* top);
static uint32_t *pmem = NULL;
bool ifbreak = false;

void ifebreak_func(int key){
	//printf("while");
	if(key == 9) {ifbreak = true; } 
}

void init_mem_npc(){
	pmem = (uint32_t *)malloc(sizeof(uint32_t)*30);
	assert(pmem);
	// *pmem = 0b000000000001 00000 000 00001 0010011; //addi    x[1] = 0 + 1 
	// *(pmem + 1) = 0b00000000010 00000 000 00001 00010011; //2   x[1] = 0 + 2 
	// *(pmem + 2) = 0b00000000011 00000 000 00001 10010011; //3	x[1] = 0 + 3 
	// *(pmem + 3) = 0b00000000100 00000 000 00010 00010011; //4	x[2] = 0 + 4 
	// *(pmem + 4) = 0b00000000101 00000 000 00010 10010011; //5	x[2] = 0 + 5 
	// *(pmem + 5) = 0b00000000111 00000 000 00011 10010011; //addi x[3] = 0 + 6 
	// *(pmem + 6) = 0b00000001000 00000 000 00100 00010011; //addi x[4] = 0 + 7
	*pmem = 0b00000000000100000000000010010011; //addi    x[1] = 0 + 1 
	*(pmem + 1) = 0b00000000010000000000000100010011; //2   x[1] = 0 + 2 
	*(pmem + 2) = 0b00000000011000000000000110010011; //3	x[1] = 0 + 3 
	*(pmem + 3) = 0b00000000100000000000001000010011; //4	x[2] = 0 + 4 
	*(pmem + 4) = 0b00000000101000000000001010010011; //5	x[2] = 0 + 5 
	*(pmem + 5) = 0b00000000111000000000001110010011; //addi x[3] = 0 + 6 
	*(pmem + 6) = 0b00000001000000000000010000010011; //addi x[4] = 0 + 7
	// *(pmem + 7) = 0b00000000000000000001010000010111; //auipc
	// *(pmem + 8) = 0b00000000000000000010010000010111; //auipc
	// *(pmem + 9) = 0b00000000000000000011010000010111; //auipc
	// *(pmem + 10) = 0b00000000000000000110000110110111; //lui
	// *(pmem + 11) = 0b00000000000000001110001110110111; //lui
	// *(pmem + 12) = 0b00000000000000011110011110110111; //lui
	*(pmem + 13) = 0b00000000000100000000000001110011; //ebreak

	return ;
}

static inline uint32_t host_read(void *addr) { 
	//printf("before host_read\n");
    return *(uint32_t *)addr;
	//printf("after host_read\n");
}


uint32_t* guest_to_host(uint32_t paddr) { 
	//printf("before  guest_to_host\n");
	return pmem + (paddr - CONFIG_MBASE) / 4; 
	//printf("after guest_to_host\n");
	}

uint32_t pmem_read_npc(uint32_t addr) {
	//printf("before mem_read_npc!\n");
	uint32_t ret = host_read(guest_to_host(addr));
	// printf("after pmem_read_npc!\n");
  return ret;
}

void get_inst(){
	if(!dut.rst) dut.inst = pmem_read_npc(dut.pc);
}


int main(int argc, char** argv, char** env) {

	//nvboard_bind_all_pins(&dut);
	//nvboard_init();

	Verilated::traceEverOn(true);
	VerilatedVcdC *m_trace = new VerilatedVcdC;  
	dut.trace(m_trace, 5);               
	m_trace->open("waveform.vcd");
	init_mem_npc();
	dut.rst = 1;
	int flag = 20;
	while (sim_time < MAX_SIM_TIME) {
//	while(1){
    // clk	1   0	1	0	1	0	1	0
	// inspc0	1	1	2	2	3	3
	// ins	0	1	1	2	2	3	3	
	//evalpc1	1	2	2	3	3			
		dut.clk ^= 1; 
		if(flag-- > 14){
			dut.eval();
			m_trace->dump(sim_time);
			sim_time++;
			continue;
		}
		// printf("pc = 0x%08x\n",dut.pc);
		// printf("before pmem_read\n");
		//dut.inst = pmem_read_npc(dut.pc);
		//get_inst();
		// printf("dut.inst = 0x%032x\n",dut.inst);
		// printf("after pmem_read\n");
		dut.eval();
		m_trace->dump(sim_time);
		sim_time++;
	//	nvboard_update();
	//	usleep(1);
		dut.rst = 0;
		//printf("flag = %d\n",flag);
		if(ifbreak) {
			printf("\nebreak!\n");
			break;
		}
	}
	free(pmem);
	pmem = NULL;
	m_trace->close();
	//delete dut;
	exit(EXIT_SUCCESS);
}


