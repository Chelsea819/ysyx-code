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
#define CONFIG_MBASE 0X80000000



static  TOP_NAME dut;
static uint32_t pmem[20] = {0};
bool ifbreak = false;

void ifebreak_func(int key){
	printf("while key = %d\n",key);
	if(key == 9) {ifbreak = true; } 
}

	// *pmem = 0b000000000001 00000 000 00001 0010011; //addi    x[1] = 0 + 1 
	// *(pmem + 1) = 0b00000000010 00000 000 00001 00010011; //2   x[1] = 0 + 2 
	// *(pmem + 2) = 0b00000000011 00000 000 00001 10010011; //3	x[1] = 0 + 3 
	// *(pmem + 3) = 0b00000000100 00000 000 00010 00010011; //4	x[2] = 0 + 4 
	// *(pmem + 4) = 0b00000000101 00000 000 00010 10010011; //5	x[2] = 0 + 5 
	// *(pmem + 5) = 0b00000000111 00000 000 00011 10010011; //addi x[3] = 0 + 6 
	// *(pmem + 6) = 0b00000001000 00000 000 00100 00010011; //addi x[4] = 0 + 7

// void init_mem_npc(){
// 	pmem = (uint32_t *)malloc(sizeof(uint32_t)*30);
// 	assert(pmem);
// 	*pmem = 0b00000000000100000000000010010011; //addi    x[1] = 0 + 1 
// 	*(pmem + 1) = 0b00000000010000000000000100010011; //2   x[1] = 0 + 2 
// 	*(pmem + 2) = 0b00000000011000000000000110010011; //3	x[1] = 0 + 3 
// 	*(pmem + 3) = 0b00000000100000000000001000010011; //4	x[2] = 0 + 4 
// 	*(pmem + 4) = 0b00000000101000000000001010010011; //5	x[2] = 0 + 5 
// 	*(pmem + 5) = 0b00000000111000000000001110010011; //addi x[3] = 0 + 6 
// 	*(pmem + 6) = 0b00000001000000000000010000010011; //addi x[4] = 0 + 7
// 	*(pmem + 7) = 0b00000000000000000001010000010111; //auipc
// 	*(pmem + 8) = 0b00000000000000000010010000010111; //auipc
// 	*(pmem + 9) = 0b00000000000000000011010000010111; //auipc
// 	// *(pmem + 10) = 0b00000000000000000110000110110111; //lui
// 	// *(pmem + 11) = 0b00000000000000001110001110110111; //lui
// 	// *(pmem + 12) = 0b00000000000000011110011110110111; //lui
// 	*(pmem + 10) = 0b00000000000100000000000001110011; //ebreak

// 	return ;
// }

void init_mem_npc(){
	pmem[0] = 0b00000000000100000000000010010011; //addi    x[1] = 0 + 1 
	pmem[1] = 0b00000000010000000000000100010011; //2   x[1] = 0 + 2 
	pmem[2] = 0b00000000011000000000000110010011; //3	x[1] = 0 + 3 
	pmem[3] = 0b00000000100000000000001000010011; //4	x[2] = 0 + 4 
	pmem[4] = 0b00000000101000000000001010010011; //5	x[2] = 0 + 5 
	pmem[5] = 0b00000000111000000000001110010011; //addi x[3] = 0 + 6 
	pmem[6] = 0b00000001000000000000010000010011; //addi x[4] = 0 + 7
	pmem[7] = 0b00000000000000000001010000010111; //auipc
	pmem[8] = 0b00000000000000000010010000010111; //auipc
	pmem[9] = 0b00000000000000000011010000010111; //auipc
	pmem[10] = 0b00000000000000000110000110110111; //lui
	pmem[11] = 0b00000000000000001110001110110111; //lui
	pmem[12] = 0b00000000000000011110011110110111; //lui
	pmem[13] = 0b00000000100000000000000011101111; //jal   x[1] = pc + 4 pc += 8 
	pmem[14] = 0b00000000000000001110001110110111; //lui
	pmem[15] = 0b00000000000000011110011110110111; //lui
	pmem[16] = 0b00000000100000000000000011101111; //jal   x[1] = pc + 4 pc += 8 
	pmem[17] = 0b00000000111000000000001110010011; //addi x[3] = 0 + 6 
	pmem[18] = 0b00000001000000000000010000010011; //addi x[4] = 0 + 7
	pmem[19] = 0b00000000000000001010000111100111; //jalr   x[1] = pc + 4 pc += 8 
	pmem[20] = 0b00000000000100000000000001110011; //ebreak

	return ;
}

static inline uint32_t host_read(void *addr) { 
	printf("before host_read addr = %p\n",addr);
    return *(uint32_t *)addr;
	printf("after host_read\n");
}


uint32_t* guest_to_host(uint32_t paddr) { 
	printf("before  guest_to_host\n");
	printf("pmem + (paddr - CONFIG_MBASE) / 4 = 0x%x\n",pmem + (paddr - CONFIG_MBASE) / 4);
	return pmem + (paddr - CONFIG_MBASE) / 4; 
	}

uint32_t pmem_read_npc(uint32_t addr) {
	printf("before mem_read_npc!\n");
	uint32_t ret = host_read(guest_to_host(addr));
	printf("after pmem_read_npc!\n");
  return ret;
}

// void get_inst(){
// 	if(dut.pc != 0) dut.inst = pmem_read_npc(dut.pc);
	
// }


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

	int n = 0;
	dut.inst = pmem_read_npc(dut.pc);
	printf("\n n = %d  pc = 0x%08x\n",++n,dut.pc);

	dut.eval();
	m_trace->dump(sim_time);
	sim_time++;

	while (sim_time < MAX_SIM_TIME) {		
		dut.clk ^= 1;
		dut.eval();
		if(dut.clk == 1) {dut.inst = pmem_read_npc(dut.pc);
		printf("\n n = %d  pc = 0x%08x\n",++n,dut.pc);
}
		dut.eval();
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


