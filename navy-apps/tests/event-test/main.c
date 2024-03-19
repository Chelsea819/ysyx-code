#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  int time = 0;
  int n = 1;
  while (1) {
    char buf[64];
    do{
            time = NDL_GetTicks();
        }while(time / 500000 < n++);
    if (NDL_PollEvent(buf, sizeof(buf))) {
      printf("receive event: [%s]\n", buf);
    }
  }
  return 0;
}
