/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/

#include "sim.h"

#include<iostream>

#include <readline/readline.h>
#include <readline/history.h>
#include <locale.h>
#include <time.h>
#include <getopt.h>
#include <regex.h>
#include "utils.h"
#include "common.h"
#include "reg.h"
#include "cpu.h"
#include "decode.h"
#include <elf.h>
#include "difftest-def.h"
#include "config.h"
#include "device-def.h"
#include "device/map.h"
#include "device/mmio.h"
#include "sdb.h"

void set_npc_state(int state, vaddr_t pc, int halt_ret);
word_t pmem_read(paddr_t addr,int len);
void pmem_write(paddr_t addr, int len, word_t data);
void init_npc(int argc,char *argv[]);
void engine_start();
vaddr_t paddr_read(paddr_t addr,int len);
void paddr_write(vaddr_t addr, vaddr_t len, word_t data);

// #define __GUEST_ISA__ riscv32

vluint64_t sim_time = 0;

TOP_NAME dut;
extern WP *head;

#ifdef CONFIG_WAVE
VerilatedVcdC *m_trace = new VerilatedVcdC;
#endif

extern "C" int pmem_read(int raddr, char wmask) {
  // 总是读取地址为`raddr & ~0x3u`的4字节返回给`rdata`
  // printf("read!\n");
  // printf("raddr = 0x%08x\n",raddr); 
  // vaddr_t rdata = paddr_read((paddr_t)(raddr & ~0x3u), 4);
  // printf("rdata = 0x%08x\n",rdata);
  int len = 0;
  switch (wmask){
      case 0x1: len = 1; break;
      case 0x3: len = 2; break;
      case 0xf: len = 4; break;
      IFDEF(CONFIG_ISA64, case 0x8: len = 8; return);
      IFDEF(CONFIG_RT_CHECK, default: assert(0));
    }
  if(raddr == CONFIG_RTC_MMIO || raddr == CONFIG_SERIAL_MMIO) { 
    // Log("Read device --- [addr: 0x%08x  len: %d]",raddr,len);  
    // time_t current_time;
    // time(&current_time); // 获取系统时间戳
    // return current_time;
  }
  return paddr_read((paddr_t)raddr, len);
}
extern "C" void pmem_write(int waddr, int wdata, char wmask) {
  // 总是往地址为`waddr & ~0x3u`的4字节按写掩码`wmask`写入`wdata`
  // `wmask`中每比特表示`wdata`中1个字节的掩码,
  // 如`wmask = 0x3`代表只写入最低2个字节, 内存中的其它字节保持不变
  // printf("pc = 0x%08x\n",dut.pc);
  // printf("wmask = 0x%01u\n",wmask);
  // printf("waddr = 0x%08x\n",(paddr_t)waddr);
  // printf("wdata = 0x%08x\n",(paddr_t)wdata);
  if(waddr == CONFIG_SERIAL_MMIO) {
    // Log("Write device --- [addr: 0x%08x data: 0x%08x]",waddr,wdata);
    // putchar(wdata);
  }
  // else {
    int len = 0;
    switch (wmask){
      case 0x1: len = 1; break;
      case 0x3: len = 2; break;
      case 0xf: len = 4; break;
      IFDEF(CONFIG_ISA64, case 0x8: len = 8; return);
      IFDEF(CONFIG_RT_CHECK, default: assert(0));
    }
    paddr_write((vaddr_t)waddr, (vaddr_t)len, (word_t)wdata);
  // }
}

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

