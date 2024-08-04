#include "npc.h"
#include "paddr.h"
#include "macro.h"
#include "host.h"

#define CONFIG_MSIZE 0x8000000
#define PG_ALIGN __attribute((aligned(4096)))

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

uint8_t* guest_to_host_npc(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
// paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read_npc(paddr_t addr,int len) {
  word_t ret = host_read_npc(guest_to_host_npc(addr), len);
  return ret;
}

static void pmem_write_npc(paddr_t addr, int len, word_t data) {
  host_write_npc(guest_to_host_npc(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
  panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
      addr, PMEM_LEFT, PMEM_RIGHT, dut.pc);
}


void init_mem_npc(){
    uint32_t *p = (uint32_t *)pmem;
    int i;
    for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
        p[i] = rand();
    }
	return ;
}

static vaddr_t load_mem_npc(paddr_t addr,int len) {
	if (likely(in_pmem(addr))) {return pmem_read_npc(addr,len);}
  // IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void mem_write_npc(vaddr_t addr, int len, word_t data) {
  // bool b = in_pmem(addr);
  // printf("addr = 0x%08x\n",addr);
  // if(b) printf("ok\n");
  if (likely(in_pmem(addr))) { return pmem_write_npc(addr, len, data);}
  // IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}