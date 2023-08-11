#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

int printf(const char *fmt, ...) {
  panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

char arr_tmp[20] = {0};
int numAdd = 19;

void convert(int num){
  int tmp = num % 10;
  arr_tmp[numAdd --] = tmp + 48; 
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);
  int flag = 1;
  int tmp = 0;
  int num = 0;
  int k = 0;
  for(int i = 0; *(fmt + i) != '\0'; i++,k++){
    if(fmt[i] == '%') {flag^=1; tmp = i;}
    if(flag == 1 || i == tmp + 1){
      if(fmt[i] == 'd'){
        memset(arr_tmp,0,20 * sizeof(char));
        numAdd = 19;
        num = va_arg(ap,int);
        while(num/flag != 0){
          convert(num/flag);
          flag*=10;
        }
        for(k = k - 1;numAdd < 20 ; k++){
          out[k] = arr_tmp[++ numAdd];
        }
      } 
      if(fmt[i] == 's'){
        char *str = va_arg(ap,char*);
        k --;
        for(int j = 0; str[j] != '\0'; j++){
          out[k] = str[j];
        }
      }
      flag = 0;
    }
    else  out[k] = fmt[i];
  }
  return k;
  //panic("Not implemented");
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
