#include <stdio.h>
#include <NDL.h>

int main() {
  NDL_Init(0);
  int time = 0;
  int n = 1;
  while (1) {
    int i = 0;
    while(i ++ < 5000000);
    char buf[64];
    if (NDL_PollEvent(buf, sizeof(buf))) {
      printf("receive event: [%s]\n", buf);
    }
  }
  return 0;
}
