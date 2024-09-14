#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>
#define REG_A0 10
static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0};
    
    switch (c->mcause) {
      case 0xb:
        if (c->GPR1 == -1) ev.event = EVENT_YIELD;    // 系统调用号的识别
        else ev.event = EVENT_SYSCALL; // 这边已经有0 和 1 了
        break;
      case 0x80000007: 
        ev.event = EVENT_IRQ_TIMER; 
        break;
      default: 
        // printf("c->mcause: [0x%08x]\n",c->mcause);
        // printf("c->mepc: [0x%08x]\n",c->mepc);
        // printf("c->mstatus: [0x%08x]\n",c->mstatus);
        ev.event = EVENT_ERROR; break;
    }
    c->mepc += 4;
    c = user_handler(ev, c);
    assert(c != NULL);
  }
  else assert(0);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));
  // printf("init mtvec!\n");
  assert(handler);
  // register event handler
  user_handler = handler;
  assert(user_handler);

  return true;
}

// kstack是栈的范围，entry是内核的入口，arg是内核线程的参数
// kcontext()要求内核线程不能从entry返回, 否则其行为是未定义的.
// 需要在kstack的底部创建一个以entry为入口的上下文结构(目前你可以先忽略arg参数), 然后返回这一结构的指针
// yield-os会调用kcontext()来创建上下文, 并把返回的指针记录到PCB的cp中
Context *kcontext(Area kstack, void (*entry)(void *), void *arg) { // 创建内核线程的上下文
  Context *con = (Context *)kstack.end - 1;
  con->mepc = (uintptr_t)entry;
  con->gpr[REG_A0] = (uintptr_t)arg;
  con->mstatus = 0x1800;
  // printf("entry = 0x%08x\n",entry);
  return con;
}

void yield() {
#ifdef __riscv_e
  // printf("riscv32e yield!\n");
  asm volatile("li a5, -1; ecall");
#else
  printf("yield!\n");
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
