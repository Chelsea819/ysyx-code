#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

void convert(int num,int* numAdd,char *arr_tmp){
  int tmp = num % 10;
  if(tmp < 0){tmp *= -1;}
  arr_tmp[(*numAdd) --] = tmp + 48; 
}


int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt); //初始化ap

  int percent = 0; //检测%
  int tmp = 0;     //存放%的下标
  int k = 0;       //out数组的下标
  for(int i = 0; *(fmt + i) != '\0'; i++){
    if(fmt[i] == '%') {percent ^= 1; tmp = i;}

    //当percent为1时进入循环,即出现奇数个`%`
    //匹配到`%`后面的格式化输出标识符
    else if(percent == 1 && i == tmp + 1){
      // %d
      if(fmt[i] == 'd'){
        char arr_tmp[100] = {0};  //存放数字转换成的字符
        int numAdd = 98;  //数组的下标 从后往前存

        int num = va_arg(ap,int); //要替换的数字
        //每一次调用va_arg()都会修改ap，这样下一次调用 就会返回下一个参数
        //va_arg(ap,type) 这个type是为了初始化一个指向目标的指针
        //如果type不匹配或者没有下一个参数，会出现随机错误

        int flag = 10; 
        //负数
        int neg = 0;
        if(num < 0){
          neg = 1;
        }  
        while(num != 0){
          //判断这个负数是否可以直接转换为正数
          if(num < 0 && num >= -2147483647) num *= -1;
          //将整型数字转换成字符串类型
          convert(num, &numAdd, arr_tmp);
          num /= flag;
        }   
        numAdd ++;
        //将数字存入out数组
        for( ;numAdd < 99 ; k++,numAdd ++){
          //负数      
          if(neg) { putch('-'); neg = 0;}
          else {putch(arr_tmp[numAdd]); }
        }
      } 

      //%s
      if(fmt[i] == 's'){
        char *str = va_arg(ap,char*);
        for(int j = 0; str[j] != '\0'; j++,k++){
          putch(str[j]);
          // out[k] = str[j];
        }
      }
      percent = 0;
    }
    else {
      putch(fmt[i]);
      k ++;
    } 
  }
  va_end(ap);
  return k + 1;


  // panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  panic("Not implemented");
}

//碰到%后，指示%的指针不变 pCurrent先前跑 找到格式化输出的格式

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt); //初始化ap

  int percent = 0; //检测%
  int tmp = 0;     //存放%的下标
  int k = 0;       //out数组的下标

  for(int i = 0; *(fmt + i) != '\0'; i++){
    if(fmt[i] == '%') {percent ^= 1; tmp = i;}

    //当percent为1时进入循环,即出现奇数个`%`
    //匹配到`%`后面的格式化输出标识符
    else if(percent == 1 && i == tmp + 1){
      // %d
      if(fmt[i] == 'd'){
        char arr_tmp[100] = {0};  //存放数字转换成的字符
        int numAdd = 98;  //数组的下标 从后往前存

        int num = va_arg(ap,int); //要替换的数字
        //每一次调用va_arg()都会修改ap，这样下一次调用 就会返回下一个参数
        //va_arg(ap,type) 这个type是为了初始化一个指向目标的指针
        //如果type不匹配或者没有下一个参数，会出现随机错误

        int flag = 10; 
        //负数
        int neg = 0;
        if(num < 0){
          neg = 1;
        }  
        while(num != 0){
          //判断这个负数是否可以直接转换为正数
          if(num < 0 && num >= -2147483647) num *= -1;
          //将整型数字转换成字符串类型
          convert(num, &numAdd, arr_tmp);
          num /= flag;
        }   
        numAdd ++;
        //将数字存入out数组
        for( ;numAdd < 99 ; k++,numAdd ++){
          //负数      
          if(neg) {out[k] = '-'; neg = 0;}
          else {out[k] = arr_tmp[numAdd]; }
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
