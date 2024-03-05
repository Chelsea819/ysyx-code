// 进程调度
#include <proc.h>

void naive_uload(PCB *pcb, const char *filename);

#define MAX_NR_PROC 4

static PCB pcb[MAX_NR_PROC] __attribute__((used)) = {};
static PCB pcb_boot = {};
PCB *current = NULL;


void switch_boot_pcb() {
  current = &pcb_boot;
}

void context_kload(PCB *pcb, void (*entry)(void *), void *arg) {
  pcb->cp = kcontext((Area) { pcb->stack, pcb + 1 }, entry, (void *)arg);
}

void hello_fun(void *arg) {
  int j = 1;
  while (1) {
    Log("Hello World from Nanos-lite with arg '%c' for the %dth time!", "?AB"[(uintptr_t)arg], j);
    j ++;
    yield();
  }
}

void init_proc() {
  // context_kload(&pcb[0], hello_fun, (void *)1L);
  // context_kload(&pcb[1], hello_fun, (void *)2L);
  switch_boot_pcb(); // 初始化current指针

  Log("Initializing processes...");

  naive_uload(NULL, NULL); // 调用加载的第一个用户程序，然后跳转到用户程序中执行
  // load program here

}

Context* schedule(Context *prev) {
  // save the context pointer
  current->cp = prev;

  // switch between pcb[0] and pcb[1]
  current = (current == &pcb[0] ? &pcb[1] : &pcb[0]);
  
  // then return the new context
  return current->cp;
}
