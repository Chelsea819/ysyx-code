#include <common.h>
void do_syscall(Context *c);
Context* schedule(Context *prev);

static Context* do_event(Event e, Context* c) {
  // printf("do_event : &c = 0x%08x\n",c);
  // printf("do_event : c = %p\n",c);
  switch (e.event) {
    case EVENT_YIELD: c = schedule(c); break;
    case EVENT_SYSCALL: do_syscall(c); break;
    case EVENT_IRQ_TIMER : printf("EVENT_IRQ_TIMER!\n");
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
