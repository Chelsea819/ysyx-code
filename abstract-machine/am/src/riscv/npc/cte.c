#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

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
      default: ev.event = EVENT_ERROR; break;
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
  printf("init mtvec!\n");
  assert(handler);
  // register event handler
  user_handler = handler;
  assert(user_handler);

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() {
#ifdef __riscv_e
  printf("riscv32e yield!\n");
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
