/*************************************************************************
  > File Name: sim_main.cpp
  > Author: Chelsea
  > Mail: 1938166340@qq.com
  > Created Time: 2023年07月13日 星期四 11时16分41秒
  > Created Time: 2023年07月13日 星期四 11时16分41秒
 ************************************************************************/

#include "device/map.h"
#include "device/mmio.h"
#include "difftest-def.h"
#include "reg.h"
#include "sdb.h"
#include "utils.h"
#include "verilated.h"
#include <config.h>
#include <cpu/cpu.h>
#include <memory/paddr.h>
#include <verilated_vcd_c.h>
int is_exit_status_bad();
void init_npc_monitor(int argc, char *argv[]);
void engine_start();

vluint64_t sim_time = 0;
VerilatedContext *contextp = new VerilatedContext;
TOP_NAME *dut = new TOP_NAME{contextp};
extern WP *head;

#ifdef CONFIG_WAVE
VerilatedVcdC *m_trace = new VerilatedVcdC;
#endif

extern "C" void flash_read(int32_t addr, int32_t *data) { assert(0); }
extern "C" void mrom_read(int32_t addr, int32_t *data) {
  *data = 0b00000000000100000000000001110011;
}

int main(int argc, char **argv, char **env) {
  Verilated::commandArgs(argc, argv);
#ifdef CONFIG_WAVE
  Verilated::traceEverOn(
      true); //设置 Verilated 追踪模式为开启,这将使得仿真期间生成波形跟踪文件
#else
  Verilated::traceEverOn(false);
#endif
  init_npc_monitor(argc, argv);
#ifdef CONFIG_WAVE
  contextp->traceEverOn(true);
  dut->trace(m_trace, 5);
  m_trace->open("waveform.vcd");
#endif
  dut->reset = 1;
  dut->eval();
  dut->clock = 0;
  dut->eval();
#ifdef CONFIG_WAVE
  m_trace->dump(sim_time);
  sim_time++;
#endif
  dut->clock = 1;
  dut->eval();
  dut->reset = 0;
  dut->eval();
#ifdef CONFIG_WAVE
  m_trace->dump(sim_time);
  sim_time++;
#endif
  /* Start engine. */
  engine_start();
  dut->final();
#ifdef CONFIG_WAVE
  m_trace->close(); //关闭波形跟踪文件
#endif
  // exit(EXIT_SUCCESS);
  return is_exit_status_bad();
}
