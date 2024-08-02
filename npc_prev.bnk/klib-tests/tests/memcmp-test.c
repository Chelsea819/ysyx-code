#include <stdint.h>
#include "../include/ktest.h"

#define N 32
uint8_t data1[N];
uint8_t data2[N];

void reset(int l,int r) {
  int i;
  //[0,l) l
  for (i = 0; i < l; i ++) {
    data1[i] = i + 1;
    data2[i] = i;
  }
  //[l,r) r-1-l+1=r-l
  for (i = l; i < r; i ++) {
    data1[i] = i;
    data2[i] = i;
  }
  //[r,N) N-1-r+1=N-r
  for (i = r; i < N; i ++) {
    data1[i] = i;
    data2[i] = i + 2;
  }
}

// 先初始化数组
// 
void test_memcmp() {
  int l, r;
  int re1,re2, re3;
  for (l = 1; l < N; l ++) {
    for (r = l + 1; r < N; r ++) {
      // putch('a');
      reset(l,r);
      re1 = memcmp(data1,data2,l);
      re2 = memcmp(data1+l,data2+l,r-l);
      re3 = memcmp(data1+r,data2+r,N-r);

        assert(re1 > 0);
        assert(re2 == 0);
        assert(re3 < 0);
    }
  }
}

int main(){
  test_memcmp();
  return 0;
}
