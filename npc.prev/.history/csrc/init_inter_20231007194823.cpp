#include "cpu.h"

void sdb_mainloop();

/* start CPU or receive commands */
void engine_start() {
#ifdef CONFIG_TARGET_AM
/* Simulate how the CPU works. */
  cpu_exec(-1);
#else
  /* Receive commands from user. */
  sdb_mainloop();
#endif
}
