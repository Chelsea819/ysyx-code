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

#define BITMASK(bits) ((1ull << (bits)) - 1)
//位抽取
#define BITS(x, hi, lo) (((x) >> (lo)) & BITMASK((hi) - (lo) + 1)) // similar to x[hi:lo] in verilog
//符号扩展
#define SEXT(x, len) ({ struct { int64_t n : len; } __x = { .n = x }; (uint64_t)__x.n; })

#define CONFIG_MSIZE 0x8000000
#define CONFIG_MBASE 0x80000000
#define CONFIG_PC_RESET_OFFSET 0x0
#define CONFIG_RT_CHECK 1

typedef MUXDEF(PMEM64, uint64_t, uint32_t) paddr_t;
typedef MUXDEF(CONFIG_ISA64, uint64_t, uint32_t) word_t;
typedef word_t vaddr_t;

#define PMEM_LEFT  ((paddr_t)CONFIG_MBASE)
#define PMEM_RIGHT ((paddr_t)CONFIG_MBASE + CONFIG_MSIZE - 1)
#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)

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
static inline word_t host_read(void *addr, int len) {
  switch (len) {
    case 1: return *(uint8_t  *)addr;
    case 2: return *(uint16_t *)addr;
    case 4: return *(uint32_t *)addr;
    IFDEF(CONFIG_ISA64, case 8: return *(uint64_t *)addr);
    default: MUXDEF(CONFIG_RT_CHECK, assert(0), return 0);
  }
}

static inline void host_write(void *addr, int len, word_t data) {
  switch (len) {
    case 1: *(uint8_t  *)addr = data; return;
    case 2: *(uint16_t *)addr = data; return;
    case 4: *(uint32_t *)addr = data; return;
    IFDEF(CONFIG_ISA64, case 8: *(uint64_t *)addr = data; return);
    IFDEF(CONFIG_RT_CHECK, default: assert(0));
  }
}

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr,int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static word_t load_mem(paddr_t addr,int len) {
	if(dut.DataSign == 0) return (pmem_read(addr,len),len * 8);
	return (SEXT(pmem_read(addr,len),len * 8));
}

void ifebreak_func(int inst){
	//printf("while key = %d\n",key);
	if(inst == 1048691) {ifbreak = true; } 
}

void mem_write(vaddr_t addr, int len, word_t data) {
  paddr_write(addr, len, data);
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
	dut.inst = pmem_read_npc(dut.pc,4);
	dut.eval();

	m_trace->dump(sim_time);
	sim_time++;

	while (sim_time < MAX_SIM_TIME) {		
		dut.clk ^= 1;
		dut.eval();
		//上升沿取指令
		if(dut.clk == 1) {
			if(dut.memWrite) mem_write(dut.ALUResult,dut.DataLen + 1,dut.storeData);
			dut.inst = pmem_read_npc(dut.pc,4);
			dut.eval();
		}
		if(dut.memToReg == 1){
			dut.ReadData = load_mem(dut.ALUResult,dut.DataLen + 1);
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



