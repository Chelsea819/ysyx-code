#ifndef __NPC_IFETCH_H__

#include "vaddr.h"

//负责取指， 取指操作的本质只不过就是一次内存的访问而已
static inline uint32_t inst_fetch(vaddr_t *pc, int len) {
  uint32_t inst = vaddr_ifetch(*pc, len);
  (*pc) += len; //renew 's->snpc'
  return inst;
}

#endif