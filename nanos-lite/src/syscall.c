#include <common.h>
#include "syscall.h"


uintptr_t sys_yield(){
  yield();
  return 0;
}

void sys_exit(){
  halt(0);
}


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  uintptr_t ret = 0;

  #ifdef CONFIG_STRACE
  char name[10] = {0};
  switch (a[0]){
    case 1: strcpy(name,"SYS_yield"); break;
    case 0: strcpy(name,"SYS_exit"); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  Log("[STRACE]: Name = [%s] arguments: [%d] ret: [%d]\n",name,a[0],ret);
  #endif
  
  switch (a[0]){
    case 1: ret = sys_yield(); break;
    case 0: sys_exit(); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->GPRx = ret;
}


