#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  int time = 0;
  int n = 1;
  while (1) {
    // for (volatile int i = 0; i < 10000; i++) ;
    char buf[64];
    if (NDL_PollEvent(buf, sizeof(buf))) {
      printf("receive event: [%s]\n", buf);
    }
  }
  return 0;
}
