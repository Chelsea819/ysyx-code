#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

static Context* (*user_handler)(Event, Context*) = NULL;

const char *regs[] = {
  "$0", "ra", "sp", "gp", "tp", "t0", "t1", "t2",
  "s0", "s1", "a0", "a1", "a2", "a3", "a4", "a5",
  "a6", "a7", "s2", "s3", "s4", "s5", "s6", "s7",
  "s8", "s9", "s10", "s11", "t3", "t4", "t5", "t6"
};

Context* __am_irq_handle(Context *c) {
  if (user_handler) {
    Event ev = {0}; // 事件初始化声明
    switch (c->mcause) {
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c); // 根据不同事件进行不同操作
    for(int i = 0; i < 32; i++){
      printf("\033[104m %d %s: \033[0m \t0x%08x\n",i,regs[i],c->gpr[i]);
    }
    printf("c->mcause: 0x%08x\n",c->mcause);
    printf("c->mstatus: 0x%08x\n",c->mstatus);
    printf("c->mepc: 0x%08x\n",c->mepc);
    assert(c != NULL);
  }

  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) { // 进行CTE相关的初始化操作
  //接受一个来自操作系统的事件处理回调函数的指针, 
  //当发生事件时, CTE将会把事件和相关的上下文作为参数, 来调用这个回调函数, 交由操作系统进行后续处理.
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap)); // 将异常入口地址设置到mtvec寄存器中

  // register event handler
  user_handler = handler; // 注册一个事件处理回调函数, 这个回调函数由yield test提供

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  return NULL;
}

void yield() { //进行自陷操作, 会触发一个编号为EVENT_YIELD事件. 不同的ISA会使用不同的自陷指令来触发自陷操作
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) { // 打开中断
}
