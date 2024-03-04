#include <common.h>

void init_mm(void);
void init_device(void);
void init_ramdisk(void);
void init_irq(void);
void init_fs(void);
void init_proc(void);

int main() {
  extern const char logo[];
  printf("%s", logo);
  Log("'Hello World!' from Nanos-lite");  // Log -> klib(printf) -> TRM(putch)
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_mm();

  init_device(); // 对设备进行初始化操作

  init_ramdisk(); // 初始化ramdisk
  /* 一般来说，程序应该放在永久存储的介质中（比如磁盘），
     但是在NEMU中对磁盘进行模拟是一个略显复杂的工作，
     因此先让nanos-lite把其中的一段内存作为磁盘来使用，这样的磁盘有一个专门的名字，叫ramdisk */

#ifdef HAS_CTE
  init_irq();
#endif

  init_fs();  // 初始化文件系统

  init_proc();  // 创建进程

  Log("Finish initialization");

#ifdef HAS_CTE
  yield();
#endif

  panic("Should not reach here");
}
