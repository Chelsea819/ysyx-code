#include "../include/ktest.h"

#define N 32
#define M 10

char src[M];
char dest[N];

void reset_strcpy(int num) {
  int i,k;
  for (i = num, k = 0; i < num + M && k < M; i ++,k++) {
    src[k] = i + 'a';
  }
}

// 检查两个字符串数组中的值是否相等
void check_eq_strcpy(char *data,char *dest) {
  int i = 0;
  for (i = 0; data[i] != '\0' && dest[i] != '\0'; i ++) {
    assert(data[i] == dest[i]);
  }
  assert(data[i] == '\0' && dest[i] == '\0');
}

void test_strcpy() {
  int l;
  for (l = 0; l < N; l ++) {
      reset_strcpy(l);
      strcpy(dest, src); // data, 0, 1
      check_eq_strcpy(src, dest); // 0 1 0
    }
}


int main(){
  test_strcpy();
  return 0;
}
