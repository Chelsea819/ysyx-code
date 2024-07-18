#include <common.h>
#include "syscall.h"
#include <sys/time.h>
int fs_open(const char *pathname, int flags, int mode);
size_t fs_read(int fd, void *buf, size_t len);
size_t fs_write(int fd, const void *buf, size_t len);
size_t fs_lseek(int fd, size_t offset, int whence);
int fs_close(int fd);
char *get_filename(int fd);

uintptr_t sys_yield(){
  yield();
  return 0;
}

void sys_exit(int status){
  halt(status);
}

/* Nanos-lite还是一个单任务操作系统, 空闲的内存都可以让用户程序自由使用, 
  因此我们只需要让SYS_brk系统调用总是返回0即可, 表示堆区大小的调整总是成功. */ 
uintptr_t sys_brk(){
  return 0;
}

uintptr_t sys_gettimeofday(struct timeval *tv, struct timezone *tz){
  // printf("sys_gettimeofday\n");
  if(tv == NULL){
    panic("struct timeval *tv can not be NULL!");
    return -1;
  }
  tv->tv_usec = io_read(AM_TIMER_UPTIME).us;
  return 0;
}

void do_syscall(Context *c) {
  // printf("do_sys c* = %p\n");
  uintptr_t a[4];
  a[0] = c->GPR1; // 系统调用的参数在a0-a5里面
  uintptr_t ret = 0;

  switch (a[0]){
    case SYS_exit: sys_exit(c->GPR2); break;
    case SYS_yield: ret = sys_yield(); break;
    case SYS_write: ret = fs_write(c->GPR2, (void *)c->GPR3, c->GPR4); break;
    case SYS_brk: ret = sys_brk(); break;
    case SYS_open: ret = fs_open((char *)c->GPR2, c->GPR3, c->GPR4); break;
    case SYS_read: ret = fs_read(c->GPR2, (void *)c->GPR3, c->GPR4); break;
    case SYS_close: ret = fs_close(c->GPR2); break;
    case SYS_lseek: ret = fs_lseek(c->GPR2, c->GPR3, c->GPR4); break;
    case SYS_gettimeofday: ret = sys_gettimeofday((struct timeval *)c->GPR2, (struct timezone *)c->GPR3); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

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
    case SYS_gettimeofday: strcpy(name,"SYS_gettimeofday"); break;
    // case x: strcpy(name,"SYS_write"); break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  // Log("[STRACE]: Name = [%s] arguments: [%d] ret: [%d]\n",name,a[0],ret);

  switch (a[0]){
    case SYS_write:
    case SYS_read:
    case SYS_close:
    case SYS_lseek:
      Log("[STRACE]: Name = [%s] Filename: [%s] arguments: [%d] ret: [%d]\n",name,get_filename(c->GPR2),a[0],ret);
      break;
    case SYS_open: 
      Log("[STRACE]: Name = [%s] Filename: [%s] arguments: [%d] ret: [%d]\n",name,(char *)(c->GPR2),a[0],ret); 
      break;
    case SYS_exit : 
    case SYS_yield: 
    case SYS_brk: 
    case SYS_gettimeofday:
      Log("[STRACE]: Name = [%s] arguments: [%d] ret: [%d]\n",name,a[0],ret);
      break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
  #endif
  c->GPRx = ret;
  // printf("[do_syscall] c->mepc = 0x%08x\n",c->mepc);
}


