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

void *memset_bk(void *s, int c, size_t n) {
  unsigned char *sp = (unsigned char *)s;
  unsigned char val = (unsigned char)c;
  for(int i = 0; i < n; i++){
    *((int *)sp + i) = val;
  }
  return s;
  //panic("Not implemented");
}


static char arr_tmp[20] = {0};
static int numAdd = 19;

void convert(int num){
  int tmp = num % 10;
  arr_tmp[numAdd --] = tmp + 48; 
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt);
  int flag = 1;    
  int percent = 0; //检测%
  int tmp = 0; 
  int num = 0; 
  int k = 0;
  for(int i = 0; *(fmt + i) != '\0'; i++,k++){
    if(fmt[i] == '%') {percent^=1; tmp = i;}
    if(percent == 1 && i == tmp + 1){
        
      if(fmt[i] == 'd'){
        memset_bk(arr_tmp,0,20 * sizeof(char));
        numAdd = 19;
        num = va_arg(ap,int);
        flag = 1;
        while(num/flag != 0){
          convert(num/flag);
          flag*=10;
        }   
        k --;
        numAdd ++;
        for( ;numAdd < 20 ; k++,numAdd ++){
          out[k] = arr_tmp[numAdd];
        }
        k--;
      } 
      if(fmt[i] == 's'){
        char *str = va_arg(ap,char*);
        k --;
        for(int j = 0; str[j] != '\0'; j++,k++){
          out[k] = str[j];
        }
        k --;
      }
      percent = 0;
    }
    else  out[k] = fmt[i];
  }
  va_end(ap);
  memset_bk(out,0,k * sizeof(char));
  return k;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
