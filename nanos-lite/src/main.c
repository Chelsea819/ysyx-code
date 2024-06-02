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
  Log("'Hello World!' from Nanos-lite");
  Log("Build time: %s, %s", __TIME__, __DATE__);

  init_mm();

  init_device();

  init_ramdisk();

#ifdef HAS_CTE
  init_irq();
#endif

  init_fs(); // 初始化文件系统

  init_proc();  // 创建进程
  printf("ok!\n");

  Log("Finish initialization");
  printf("ok!\n");

#ifdef HAS_CTE
  yield();
#endif

  panic("Should not reach here");
}
