/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NPC is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <cstdio>
#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>
#include <common.h>
#include <debug.h>
#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
//{}使用聚合初始化把数组初始化为0
#endif

extern TOP_NAME *dut; extern VerilatedVcdC *m_trace;
uint8_t* guest_to_host(paddr_t paddr) { 
  // printf("pmem: 0x%08x\n",pmem);
  // printf("paddr: 0x%08x\n",paddr);
  // printf("CONFIG_MBASE: 0x%08x\n",CONFIG_MBASE);
  return pmem + paddr - CONFIG_MBASE; 
}
// paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

extern "C" int pmem_read_task(int raddr, char wmask) {
  // 总是读取地址为`raddr & ~0x3u`的4字节返回给`rdata`
  // printf("read!\n");
  // printf("raddr = 0x%08x\n",raddr); 
  // vaddr_t rdata = paddr_read((paddr_t)(raddr & ~0x3u), 4);
  if (wmask == 0) {
    return 0;
  }
  int len = 0;
  switch (wmask){
      case 0x1: len = 1; break;
      case 0x3: len = 2; break;
      case 0xf: len = 4; break;
      IFDEF(CONFIG_ISA64, case 0x8: len = 8; return);
      IFDEF(CONFIG_RT_CHECK, default: assert(0));
    }
  #ifdef CONFIG_DEVICE
    #ifdef CONFIG_RTC_MMIO 
    if(raddr == CONFIG_RTC_MMIO) { 
      // Log("Read device --- [addr: 0x%08x  len: %d]",raddr,len);  
      // time_t current_time;
      // time(&current_time); // 获取系统时间戳
      // return current_time;
    }
    #endif
    #ifdef CONFIG_SERIAL_MMIO 
    if(raddr == CONFIG_SERIAL_MMIO) { 
      // Log("Read device --- [addr: 0x%08x  len: %d]",raddr,len);  
      // time_t current_time;
      // time(&current_time); // 获取系统时间戳
      // return current_time;
    }
    #endif
  #endif
  return paddr_read((paddr_t)raddr, len);
}
extern "C" void pmem_write_task(int waddr, int wdata, char wmask) {
  // 总是往地址为`waddr & ~0x3u`的4字节按写掩码`wmask`写入`wdata`
  // `wmask`中每比特表示`wdata`中1个字节的掩码,
  // 如`wmask = 0x3`代表只写入最低2个字节, 内存中的其它字节保持不变
  // printf("pc = 0x%08x\n",dut->pc);
  // printf("wmask = 0x%01u\n",wmask);
  // printf("waddr = 0x%08x\n",(paddr_t)waddr);
  // printf("wdata = 0x%08x\n",(paddr_t)wdata);

  #ifdef CONFIG_DEVICE
    #ifdef CONFIG_SERIAL_MMIO 
    if(waddr == CONFIG_SERIAL_MMIO) {
    // Log("Write device --- [addr: 0x%08x data: 0x%08x]",waddr,wdata);
    // putchar(wdata);
  }
    #endif
  #endif
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

static word_t pmem_read(paddr_t addr,int len) {
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, dut->pc);
}

void init_mem(){
    uint32_t *p = (uint32_t *)pmem;
    int i;
    for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
        p[i] = 0;
    }
	return ;
}

vaddr_t paddr_read(paddr_t addr,int len) {
  
	if (likely(in_pmem(addr))) {
    word_t rdata = pmem_read(addr,len);
    // #ifdef CONFIG_MTRACE
    //   Log("paddr_read ---  [addr: 0x%08x len: %d rdata: 0x%08x]",addr,len,rdata);
    // #endif
    return rdata;
  }
  // printf("read device\n");
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  // printf("read device---out of bound\n");
  out_of_bound(addr);
  return 0;
}

void paddr_write(vaddr_t addr, vaddr_t len, word_t data) {
  #ifdef CONFIG_MTRACE
  Log("paddr_write --- [addr: 0x%08x len: %d data: 0x%08x]",addr,len,data);
  #endif
  if (likely(in_pmem(addr))) { return pmem_write(addr, len, data);}
  // printf("write device\n");

  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  // printf("paddr_write device---out of bound\n");

  out_of_bound(addr);
}
