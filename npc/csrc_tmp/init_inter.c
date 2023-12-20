#include "cpu.h"

void sdb_mainloop();

/* start CPU or receive commands */
void engine_start() {
  /* Receive commands from user. */
  sdb_mainloop();
}
