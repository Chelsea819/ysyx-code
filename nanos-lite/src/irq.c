#include <common.h>
void do_syscall(Context *c);

static Context* do_event(Event e, Context* c) {
  // printf("do_event : &c = 0x%08x\n",&c);
  printf("do_event : c = %p\n",c);
  printf("1s\n");
  switch (e.event) {
    case EVENT_YIELD: printf("Yield!\n"); break;
    case EVENT_SYSCALL: do_syscall(c); printf("[do_event] c->mepc = 0x%08x\n",c->mepc);break;
    default: panic("Unhandled event ID = %d", e.event);
  }
  // printf("do_event_after : c = %p\n",c);
  // printf("[do_event] c->mepc = 0x%08x\n",c->mepc);
  return c;
}

void init_irq(void) {
  Log("Initializing interrupt/exception handler...");
  cte_init(do_event);
}
