#include <common.h>
#include "syscall.h"


uintptr_t sys_yield(){
  yield();
  return 0;
}

void sys_exit(){
  halt(0);
}

uintptr_t sys_write(Context *c){
  // 检查fd的值
  int i = 0;
  if(c->GPR2 == 1 || c->GPR2 == 2){
    for(i = 0; i < c->GPR4; i ++){
      // putch('a');
      if(!c->GPR3) return -1;
      putch(*((char *)c->GPR3 + i));
    }
  } 
  // putch('b');
  return i;
}

uintptr_t sys_brk(){
  return 0;
}


void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1;
  uintptr_t ret = 0;

  #ifdef CONFIG_STRACE
  char name[10] = {0};
  switch (a[0]){
    case SYS_exit : strcpy(name,"SYS_exit"); break;
    case SYS_yield: strcpy(name,"SYS_yield"); break;
    case SYS_write: strcpy(name,"SYS_write"); break;
    case SYS_open: strcpy(name,"SYS_open"); break;
    case SYS_read: strcpy(name,"SYS_read"); break;
    case SYS_close: strcpy(name,"SYS_close"); break;
    case SYS_lseek: strcpy(name,"SYS_lseek"); break;
    case SYS_brk: strcpy(name,"SYS_brk"); break;

    // case x: strcpy(name,"SYS_write"); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  Log("[STRACE]: Name = [%s] arguments: [%d] ret: [%d]\n",name,a[0],ret);
  #endif

  switch (a[0]){
    case SYS_exit: sys_exit(); break;
    case SYS_yield: ret = sys_yield(); break;
    case SYS_write: ret = sys_write(c); break;
    case SYS_brk: ret = sys_brk(); break;
    // case SYS_open: ret = fs_open(); break;
    // case SYS_read: ret = fs_read(); break;
    // case SYS_close: ret = fs_close(); break;
    // case SYS_lseek: ret = fs_lseek(); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  c->GPRx = ret;
}


