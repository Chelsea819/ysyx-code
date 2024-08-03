
#ifndef _NPC_PADDR_H_
#define _NPC_PADDR_H_

#include <stdint.h>
#include <inttypes.h>
#include <assert.h>
#include <stdio.h>
#include "paddr.h"
#include "common.h"

#define CONFIG_MBASE 0x80000000
#define CONFIG_MSIZE 0x8000000
#define CONFIG_PC_RESET_OFFSET 0x0
#define PMEM_LEFT  ((paddr_t)CONFIG_MBASE)
#define PMEM_RIGHT ((paddr_t)CONFIG_MBASE + CONFIG_MSIZE - 1)
#define RESET_VECTOR (PMEM_LEFT + CONFIG_PC_RESET_OFFSET)

uint8_t* guest_to_host_npc(paddr_t paddr);
// paddr_t host_to_guest(uint8_t *haddr);

static inline bool in_pmem(paddr_t addr) {
  return addr - CONFIG_MBASE < CONFIG_MSIZE;
}

static vaddr_t load_mem_npc(paddr_t addr,int len);
void mem_write_npc(vaddr_t addr, int len, word_t data);

#endif