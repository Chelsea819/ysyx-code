#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)
#define MAX_SIZE 1024*4
#define NUM_BUF 50
// -2147483648
void convert(int num,int* numAdd,char *arr_tmp,int flag){
  int tmp = num % flag; //-8
  if(tmp < 0){tmp *= -1;}
  arr_tmp[(*numAdd)] = tmp + 48; 
  // putch(arr_tmp[(*numAdd)]);
  // putch('\n');
  (*numAdd) --;
}

void intHandel(int *neg, int *num, int *numAdd, int flag, char *arr_tmp){
  //负数
  if(*num < 0){
    *neg = 1;
  }  
  do{
    //判断这个负数是否可以直接转换为正数
    if(*num < 0 && *num >= -2147483647) *num *= -1;
    //将整型数字转换成字符串类型
    convert(*num, numAdd, arr_tmp,flag);
    *num /= flag;
  }while(*num != 0);   
  (*numAdd) ++;
}

void formatHandel(bool *if_wid, bool *if_for, int numAdd, int *width, char *out, int *k){
  if(*if_wid && NUM_BUF - numAdd - 1 < *width){
    for(int i = *width - (NUM_BUF - numAdd - 1); i > 0 ; i --, (*k) ++)
      if(*if_for) out[*k] = '0';
      else out[*k] = ' ';
    *if_for = false;
    *if_wid = false;
    *width = 0;
  }
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  // panic("Not implemented");
  int percent = 0; //检测%
  int tmp = 0;     //存放%的下标
  int k = 0;       //out数组的下标
  int flag = 0;
  int width = 0;
  int neg = 0;
  bool if_wid = false;
  bool if_for = false;
  bool if_long = false;

  for(int i = 0; k < n-1 && *(fmt + i) != '\0'; i++){
    if(fmt[i] == '%') {putch('%');percent ^= 1; tmp = i;}

    //当percent为1时进入循环,即出现奇数个`%`
    //匹配到`%`后面的格式化输出标识符
    else if(percent == 1 && i == tmp + 1){
      if(fmt[i] == '0' && !if_for && !if_wid){
        putch('f');
        if_for = true;
      }
      else if(fmt[i] >= '0' && fmt[i] <= '9' && !if_wid){
        putch('w');
        if(!if_wid)
          width = fmt[i] - '0';
        else
          width = width * 10 + (fmt[i] - '0');
        if_wid = true;
      }
      else if(if_for && !if_wid)
        panic("Invalid format");
      else if(fmt[i] == 'l'){
        if_long = true;
      }
      // %d
      else if(fmt[i] == 'd'){
        char arr_tmp[NUM_BUF] = {0};  //存放数字转换成的字符
        int numAdd = NUM_BUF - 2;  //数组的下标 从后往前存

        int num = va_arg(ap,int); //要替换的数字
        //每一次调用va_arg()都会修改ap，这样下一次调用 就会返回下一个参数
        //va_arg(ap,type) 这个type是为了初始化一个指向目标的指针
        //如果type不匹配或者没有下一个参数，会出现随机错误
        flag = 10; 
        neg = 0;
        intHandel(&neg, &num, &numAdd, flag, arr_tmp);
        formatHandel(&if_wid, &if_for, numAdd, &width, out, &k);
        //将数字存入out数组
        for( ; k < n-1 && numAdd < NUM_BUF - 1 ; k++,numAdd ++){
          //负数      
          if(neg) {out[k] = '-'; neg = 0;}
          else {out[k] = arr_tmp[numAdd]; }
        }
        if(if_long) if_long = false;
      } 

      //%s
      else if(fmt[i] == 's'){
        char *str = va_arg(ap,char*);
        for(int j = 0; k < n-1 && str[j] != '\0'; j++,k++){
          out[k] = str[j];
        }
      }

      // %c
      else if(fmt[i] == 'c'){
        char c = va_arg(ap,int);
        out[k] = c;
        k ++;
      }

      else if(fmt[i] == 'x'){
        char arr_tmp[NUM_BUF] = {0};  //存放数字转换成的字符
        int numAdd = NUM_BUF - 2;  //数组的下标 从后往前存

        int num = va_arg(ap,int); //要替换的数字
        //每一次调用va_arg()都会修改ap，这样下一次调用 就会返回下一个参数
        //va_arg(ap,type) 这个type是为了初始化一个指向目标的指针
        //如果type不匹配或者没有下一个参数，会出现随机错误

        flag = 16; 
        //负数
        neg = 0;
        intHandel(&neg, &num, &numAdd, flag, arr_tmp);
        formatHandel(&if_wid, &if_for, numAdd, &width, out, &k);

        //将数字存入out数组
        for( ; k < n-1 && numAdd < NUM_BUF - 1 ; k++,numAdd ++){
          //负数      
          if(neg) {out[k] = '-'; neg = 0;}
          else {out[k] = arr_tmp[numAdd]; }
        }
        if(if_long) if_long = false;
      }
      else {
        putch(fmt[i]);
        panic("Not completed format");
        return -1;
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
  return k;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt); //初始化ap
  char out[MAX_SIZE];
  int ret = vsprintf(out, fmt, ap);
  assert(ret <= MAX_SIZE && ret >= 0);
  for(int i = 0; i <= ret; i ++){
    putch(out[i]);
  }
  va_end(ap);
  return ret;
  // panic("Not implemented");
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  // panic("Not implemented");
  int ret = vsnprintf(out, MAX_SIZE, fmt, ap);
  return ret;
}

//碰到%后，指示%的指针不变 pCurrent先前跑 找到格式化输出的格式

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt); //初始化ap

  int ret = vsprintf(out, fmt, ap);

  va_end(ap);
  return ret;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  // panic("Not implemented");
  va_list ap;
  va_start(ap,fmt); //初始化ap

  int ret = vsnprintf(out, n, fmt, ap);

  va_end(ap);
  return ret;
}


#endif
