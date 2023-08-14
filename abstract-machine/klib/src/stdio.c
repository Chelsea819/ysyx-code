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


void convert(int num,int* numAdd,char *arr_tmp){
  int tmp = num % 10;
  arr_tmp[(*numAdd) --] = tmp + 48; 
}

// int sprintf(char *out, const char *fmt, ...) {
//   char arr_tmp[20] = {0};  //存放数字转换成的字符
//   int numAdd = 18;  //数组的下标

//   va_list ap;
//   va_start(ap,fmt);

//   int flag = 1;    //位数
//   int percent = 0; //检测%
//   int tmp = 0;     //存放%的下标
//   int num = 0;     //
//   int k = 0;
//   for(int i = 0; *(fmt + i) != '\0'; i++,k++){
//     if(fmt[i] == '%') {percent^=1; tmp = i;}
//     if(percent == 1 && i == tmp + 1){
//       if(fmt[i] == 'd'){
//         //memset_bk(arr_tmp,0,20 * sizeof(char));
//         numAdd = 18;
//         num = va_arg(ap,int);
//         flag = 1;
//         while(num/flag != 0){
//           convert(num/flag,&numAdd,arr_tmp);
//           flag*=10;
//         }   
//         k --;
//         numAdd ++;
//         for( ;numAdd < 19 ; k++,numAdd ++){
//           out[k] = arr_tmp[numAdd];
//         }
//         k--;
//       } 
//       if(fmt[i] == 's'){
//         char *str = va_arg(ap,char*);
//         k --;
//         for(int j = 0; str[j] != '\0'; j++,k++){
//           out[k] = str[j];
//         }
//         k --;
//       }
//       percent = 0;
//     }
//     else  out[k] = fmt[i];
//   }
//   out[k + 1] = '\0';
//   va_end(ap);
//   return k + 1;
// }


//碰到%后，指示%的指针不变 pCurrent先前跑 找到格式化输出的格式

int sprintf(char *out, const char *fmt, ...) {
  
  va_list ap;
  va_start(ap,fmt);

  putch('a');
  int percent = 0; //检测%
  int tmp = 0;     //存放%的下标
  int k = 0;       //out数组的下标

  for(int i = 0; *(fmt + i) != '\0'; i++){
    if(fmt[i] == '%') {percent^=1; tmp = i;}

    //当percent为1时进入循环
    else if(percent == 1 && i == tmp + 1){
      // %d
      if(fmt[i] == 'd'){
        char arr_tmp[20] = {0};  //存放数字转换成的字符
        int numAdd = 18;  //数组的下标
        int num = va_arg(ap,int); //要替换的数字
        int flag = 1;  //位数
        while(num/flag != 0){
          convert(num/flag,&numAdd,arr_tmp);
          flag *= 10;
        }   
        numAdd ++;
        for( ;numAdd < 19 ; k++,numAdd ++){
          out[k] = arr_tmp[numAdd];
        }
      } 

      //%s
      if(fmt[i] == 's'){
        char *str = va_arg(ap,char*);
        for(int j = 0; str[j] != '\0'; j++,k++){
          out[k] = str[j];
        }
      }
      percent = 0;
    }
    else {
      out[k] = fmt[i];
      k++;
    } 
  }
  out[k] = '\0';
  va_end(ap);
  return k + 1;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif
