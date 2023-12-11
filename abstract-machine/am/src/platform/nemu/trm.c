#include <am.h>
#include <nemu.h>

//支撑程序运行在TRM上的API

extern char _heap_start;
int main(const char *args);

// Area:Memory area for [@start, @end]
// 用于指示堆区的起始和末尾
Area heap = RANGE(&_heap_start, PMEM_END);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;

//用于输出一个字符
void putch(char ch) {
  outb(SERIAL_PORT, ch);
}

//用于结束程序的运行
void halt(int code) {
  nemu_trap(code);

  // should not reach here
  while (1);
}

//用于进行TRM相关的初始化工作
void _trm_init() {
  int ret = main(mainargs);
  halt(ret);
}
