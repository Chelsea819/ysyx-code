#include <am.h>
#include <npc.h>
#include "../spike/htif.h"

extern char _heap_start;
int main(const char *args);

Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

void putch(char ch) {
    *(volatile uint8_t  *)SERIAL_PORT = ch;
}

void halt(int code) {
  //htif_poweroff();
  //exit(0);
  // asm volatile：这个关键字告诉编译器，嵌入的汇编代码不要对内联汇编代码进行优化
  asm volatile ("ebreak");
  while (1);
}

void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
