#include <stdint.h>
#include "../include/ktest.h"

#define N 32
uint8_t data[N];

void reset() {
  int i;
  for (i = 0; i < N; i ++) {
    data[i] = i + 1;
  }
}

// 检查[l,r)区间中的值是否依次为val, val + 1, val + 2...
void check_seq(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    assert(data[i] == val + i - l);
  }
}

// 检查[l,r)区间中的值是否均为val
void check_eq(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    assert(data[i] == val);
  }
}

void test_memset() {
  int l, r;
  for (l = 0; l < N; l ++) {
    //l = 0
    for (r = l + 1; r <= N; r ++) {
      //l = 0 r = 1 r <= 32
      reset();
      // 1 2 3 4 ……
      uint8_t val = (l + r) / 2;
      memset(data + l, val, r - l); // data, 0, 1
      check_seq(0, l, 1);  // 0, 0, 1
      check_eq(l, r, val); // 0 1 0
      check_seq(r, N, r + 1); //1 32 2
    }
  }
}

int main(){
  test_memset();
  return 0;
}
