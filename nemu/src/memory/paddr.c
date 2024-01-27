/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
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

#include <memory/host.h>
#include <memory/paddr.h>
#include <device/mmio.h>
#include <isa.h>

#if   defined(CONFIG_PMEM_MALLOC)
static uint8_t *pmem = NULL;
#else // CONFIG_PMEM_GARRAY
static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};
//{}使用聚合初始化把数组初始化为0
#endif

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  
  word_t ret = host_read(guest_to_host(addr), len);
  #ifdef CONFIG_MTRACE
      Log("paddr_read ---  [addr: 0x%08x len: %d rdata: 0x%08x]",addr,len,ret);
  #endif
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  #ifdef CONFIG_MTRACE
  Log("paddr_write --- [addr: 0x%08x len: %d data: 0x%08x]",addr,len,data);
  #endif
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
}

void init_mem() {
#if   defined(CONFIG_PMEM_MALLOC)
  pmem = malloc(CONFIG_MSIZE);
  /* 如果定义了CONFIG_PMEM_MALLOC,
  则调用malloc()分配一块CONFIG_MSIZE大小的内存区域,
  并存入pmem变量 */
  assert(pmem);
  //检查分配是否成功。
#endif
#ifdef CONFIG_MEM_RANDOM
  /*如果定义了CONFIG_MEM_RANDOM,
  则将pmem作为uint32_t指针,
  循环初始化为随机数*/
  uint32_t *p = (uint32_t *)pmem;
  int i;
  for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
    p[i] = rand();
  }
#endif
  /*打印日志,输出初始化的内存区域的地址范围,
  FMT_PADDR是地址格式化的宏*/
  /*PMEM_LEFT和PMEM_RIGHT应该是
  预定义的内存区域起始和结束地址*/
  Log("physical memory area [" FMT_PADDR ", " FMT_PADDR "]", PMEM_LEFT, PMEM_RIGHT);
}

word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len); // 地址落在物理内存空间
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));  // 地址落在设备空间
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}
