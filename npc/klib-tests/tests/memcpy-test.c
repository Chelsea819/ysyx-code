#include "../include/ktest.h"
#include <stdint.h>

#define N 32
uint8_t data[N];

void reset_memcpy() {
  int i;
  for (i = 0; i < N; i ++) {
    data[i] = i + 1;
  }
}

// 检查[l,r)区间中的值是否依次为val, val + 1, val + 2...
void check_seq_memcpy(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    assert(data[i] == val + i - l);
  }
}

// 检查[l,r)区间中的值是否均为val
void check_eq_memcpy(int l, int r, int val) {
  int i;
  for (i = l; i < r; i ++) {
    assert(data[i] == val);
  }
}

void test_memcpy() {
  int l, r;
  for (l = 0; l < N; l ++) {
    //l = 0
    for (r = l + 1; r <= N; r ++) {
      //l = 0 r = 1 r <= 32
      reset_memcpy();
      // 1 2 3 4 ……
      uint8_t val = (l + r) / 2;
      memset(data + l, val, r - l); // data, 0, 1
      check_seq_memcpy(0, l, 1);  // 0, 0, 1
      check_eq_memcpy(l, r, val); // 0 1 0
      check_seq_memcpy(r, N, r + 1); //1 32 2
    }
  }
}

int main(){
  test_memcpy();
  return 0;
}

