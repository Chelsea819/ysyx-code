#ifndef _NPCIFETCH_H_
#define _NPCIFETCH_H_

#include "vaddr.h"

//负责取指， 取指操作的本质只不过就是一次内存的访问而已
static inline uint32_t inst_fetch(vaddr_t pc, int len) {
  uint32_t inst = vaddr_ifetch(pc, len);
  return inst;
}

#endif