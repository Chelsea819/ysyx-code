#include "npc.h"
#include "paddr.h"
#include "macro.h"
#include "host.h"

#define CONFIG_MSIZE 0x8000000
#define PG_ALIGN __attribute((aligned(4096)))

static uint8_t pmem[CONFIG_MSIZE] PG_ALIGN = {};

uint8_t* guest_to_host(paddr_t paddr) { return pmem + paddr - CONFIG_MBASE; }
paddr_t host_to_guest(uint8_t *haddr) { return haddr - pmem + CONFIG_MBASE; }

static word_t pmem_read(paddr_t addr, int len) {
  #ifdef CONFIG_MTRACE
  Log("pmem_read ---  [ %08x ]",addr);
  #endif
  word_t ret = host_read(guest_to_host(addr), len);
  return ret;
}

static void pmem_write(paddr_t addr, int len, word_t data) {
  #ifdef CONFIG_MTRACE
  Log("pmem_write --- [ %08x ]",addr);
  #endif
  host_write(guest_to_host(addr), len, data);
}

static void out_of_bound(paddr_t addr) {
//   panic("address = " FMT_PADDR " is out of bound of pmem [" FMT_PADDR ", " FMT_PADDR "] at pc = " FMT_WORD,
//       addr, PMEM_LEFT, PMEM_RIGHT, cpu.pc);
    printf("\naddress =  is out of bound of pmem\n");
}


void init_mem_npc(){
    uint32_t *p = (uint32_t *)pmem;
    int i;
    for (i = 0; i < (int) (CONFIG_MSIZE / sizeof(p[0])); i ++) {
        p[i] = rand();
    }
	return ;
}



word_t paddr_read(paddr_t addr, int len) {
  if (likely(in_pmem(addr))) return pmem_read(addr, len);
  IFDEF(CONFIG_DEVICE, return mmio_read(addr, len));
  out_of_bound(addr);
  return 0;
}

void paddr_write(paddr_t addr, int len, word_t data) {
  if (likely(in_pmem(addr))) { pmem_write(addr, len, data); return; }
  IFDEF(CONFIG_DEVICE, mmio_write(addr, len, data); return);
  out_of_bound(addr);
}