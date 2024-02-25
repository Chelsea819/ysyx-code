#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>
// #include <stdio.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define MAX_SIZE 1024*4
#define NUM_BUF 50
// -2147483648
void convert1(int num,int* numAdd,char *arr_tmp,int flag){
  int tmp = num % flag; //-8
  if(tmp < 0){tmp *= -1;}
  arr_tmp[(*numAdd)] = tmp + 48; 
  // putch(arr_tmp[(*numAdd)]);
  // putch('\n');
  (*numAdd) --;
}

void intHandel1(int *neg, int *num, int *numAdd, int flag, char *arr_tmp){
  //负数
  if(*num < 0){
    *neg = 1;
  }  
  do{
    //判断这个负数是否可以直接转换为正数
    if(*num < 0 && *num >= -2147483647) *num *= -1;
    //将整型数字转换成字符串类型
    convert1(*num, numAdd, arr_tmp,flag);
    *num /= flag;
  }while(*num != 0);   
  (*numAdd) ++;
}

void formatHandel1(bool *if_wid, bool *if_for, int numAdd, int *width, char *out, int *k){
  if(*if_wid && NUM_BUF - numAdd - 1 < *width){
    for(int i = *width - (NUM_BUF - numAdd - 1); i > 0 ; i --, (*k) ++)
      if(*if_for) out[*k] = '0';
      else out[*k] = ' ';
    *if_for = false;
    *if_wid = false;
    *width = 0;
  }
}

int vsnprintf1(char *out, size_t n, const char *fmt, va_list ap) {
  // panic("Not implemented");
  int percent = 0; //检测%
  // int tmp = 0;     //存放%的下标
  int k = 0;       //out数组的下标
  int flag = 0;
  int width = 0;
  int neg = 0;
  bool if_wid = false;
  bool if_for = false;
  bool if_long = false;

  for(int i = 0; k < n-1 && *(fmt + i) != '\0'; i++){
    // if(fmt[i] == '%') {putch(fmt[i]);percent ^= 1; tmp = i;}
    if(fmt[i] == '%') {percent ^= 1; }

    //当percent为1时进入循环,即出现奇数个`%`
    //匹配到`%`后面的格式化输出标识符
    else if(percent == 1){
      // putch(fmt[i]);
      //%s
      if(fmt[i] == 's'){
        percent = 0;
        char *str = va_arg(ap,char*);
        for(int j = 0; k < n-1 && str[j] != '\0'; j++,k++){
          // putch(fmt[i]);
          out[k] = str[j];
        }
      }

      // %c
      else if(fmt[i] == 'c'){
        // putch(fmt[i]);
        percent = 0;
        char c = va_arg(ap,int);
        out[k] = c;
        k ++;
      }
      else if(fmt[i] == '0' && !if_for && !if_wid){
        // putch('f');
        if_for = true;
      }
      else if(fmt[i] >= '0' && fmt[i] <= '9' && !if_wid){
        // putch('w');
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
        // putch(fmt[i]);
        percent = 0;
        char arr_tmp[NUM_BUF] = {0};  //存放数字转换成的字符
        int numAdd = NUM_BUF - 2;  //数组的下标 从后往前存

        int num = va_arg(ap,int); //要替换的数字
        //每一次调用va_arg()都会修改ap，这样下一次调用 就会返回下一个参数
        //va_arg(ap,type) 这个type是为了初始化一个指向目标的指针
        //如果type不匹配或者没有下一个参数，会出现随机错误
        flag = 10; 
        neg = 0;
        intHandel1(&neg, &num, &numAdd, flag, arr_tmp);
        formatHandel1(&if_wid, &if_for, numAdd, &width, out, &k);
        //将数字存入out数组
        for( ; k < n-1 && numAdd < NUM_BUF - 1 ; k++,numAdd ++){
          //负数      
          if(neg) {out[k] = '-'; neg = 0;}
          else {out[k] = arr_tmp[numAdd]; }
        }
        if(if_long) if_long = false;
      } 

      

      else if(fmt[i] == 'x'){
        percent = 0;
        char arr_tmp[NUM_BUF] = {0};  //存放数字转换成的字符
        int numAdd = NUM_BUF - 2;  //数组的下标 从后往前存

        int num = va_arg(ap,int); //要替换的数字
        //每一次调用va_arg()都会修改ap，这样下一次调用 就会返回下一个参数
        //va_arg(ap,type) 这个type是为了初始化一个指向目标的指针
        //如果type不匹配或者没有下一个参数，会出现随机错误

        flag = 16; 
        //负数
        neg = 0;
        intHandel1(&neg, &num, &numAdd, flag, arr_tmp);
        formatHandel1(&if_wid, &if_for, numAdd, &width, out, &k);

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
      
    }
    else {
      // putch(fmt[i]);
      out[k] = fmt[i];
      k++;
    } 
  }
  out[k] = '\0';
  va_end(ap);
  return k;
}

int vsprintf1(char *out, const char *fmt, va_list ap) {
  // panic("Not implemented");
  int ret = vsnprintf1(out, MAX_SIZE, fmt, ap);
  return ret;
}


int printf1(const char *fmt, ...) {
  va_list ap;
  va_start(ap,fmt); //初始化ap
  char out[MAX_SIZE];
  int ret = vsprintf1(out, fmt, ap);
  assert(ret <= MAX_SIZE && ret >= 0);
  for(int i = 0; i <= ret; i ++){
    putch(out[i]);
  }
  va_end(ap);
  return ret;
  // panic("Not implemented");
}


/*calculates  the length of the string 
 pointed to by s, excluding 
 the terminating null byte ('\0').*/
/*returns the number of bytes in
       the string pointed to by s.*/
size_t strlen(const char *s) {
  size_t length = 0;
  for(int i = 0; s[i] != '\0'; i++){
    length ++;
  }
  return length;
  //panic("Not implemented");
}

/*return a pointer
       to the destination string dest.*/
/*copies the string pointed to by
       src, including the terminating null  byte  ('\0'),  to
       the  buffer  pointed  to by dest.*/

char *strcpy(char *dst, const char *src) {
  // assert(strlen(src) > strlen(dst));
  int i = 0;
  for(i = 0; src[i] != '\0'; i++){
    dst[i] = src[i];
  }
  dst[i] = '\0';
  return dst;
  //panic("Not implemented");
}

char *strncpy(char *dst, const char *src, size_t n) {
  int i = 0;
  for(i = 0; i < n ; i ++){
    dst[i] = src[i];
  }
  for(; i < n; i++){
    dst[i] = '\0';
  }
   return dst;
  //panic("Not implemented");
}

char *strcat(char *dst, const char *src) {
  int i = 0;
  int k = 0;
  for(i = 0; dst[i] != '\0'; i++);
  for(; src[k] != '\0'; i++ ,k++){
    dst[i] = src[k];
  }
  dst[i] = '\0';
  return dst;
  //panic("Not implemented");
}

int strcmp(const char *s1, const char *s2) {
  int i = 0;
  for(i = 0; s1[i] != '\0'&& s2[i] != '\0' ; i ++){
    if((unsigned char)s1[i] == (unsigned char)s2[i]) continue;
    if((unsigned char)s1[i] > (unsigned char)s2[i]) return 1;
    if((unsigned char)s1[i] < (unsigned char)s2[i]) return -1;
  }
  size_t len1 = strlen(s1);
  size_t len2 = strlen(s2);
  if(len1 > len2) return 1;
  else if(len1 < len2) return -1;
  return 0;
  //panic("Not implemented");
}

int strncmp(const char *s1, const char *s2, size_t n) {
  int i = 0;
  for(i = 0; s1[i] != '\0'&& s2[i] != '\0' && i < n; i ++){
    if((unsigned char)s1[i] == (unsigned char)s2[i]) continue;
    if((unsigned char)s1[i] > (unsigned char)s2[i]) return 1;
    if((unsigned char)s1[i] < (unsigned char)s2[i]) return -1;
  }
  if(s1[i] == '\0') return -1;
  if(s2[i] == '\0') return 1;
  return 0;
  //panic("Not implemented");
}

/* fill memory with a constant byte */
/* The memset() function returns a pointer 
      to the memory area s. */
void *memset(void *s, int c, size_t n) {
  printf1("n = %s\n",n);
  unsigned char *sp = (unsigned char *)s;
  unsigned char val = (unsigned char)c;
  for(size_t i = 0; i < n; i++){
    *(sp + i) = val;
  }
  return s;
  //panic("Not implemented");
}

/* The  memmove() function copies n bytes from memory area src to memory area dest.
    The memory areas may overlap: copying takes place as though the bytes in src are
    first  copied  into a temporary array that does not overlap src or dest, and the
    bytes are then copied from the temporary array to dest. */

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *in = (unsigned char*)src;
  unsigned char *out = (unsigned char*)dst;
  for(int k = 0; k < n; k ++){
    out[k] = in[k];
  //panic("Not implemented");
  }
  return dst;
}

//根据两个区间之间的关系，选择合适的copy方式
//区间关系不同，copy方向不同
void *memcpy(void *out, const void *in, size_t n) {
  unsigned char *dst = (unsigned char *)out;
  unsigned char *src = (unsigned char *)in;
  if(src <= dst && (src + n) >= dst){
    for(int k = n-1; k >= 0; k --){
      dst[k] = src[k];
    }
  }
  else{
    for(int k = 0; k < n; k ++){
      dst[k] = src[k];
    }
  }
  return out;
  //panic("Not implemented");
}

int memcmp(const void *s1, const void *s2, size_t n) {
  for(int i = 0; i < n ; i ++){
    if(*((unsigned char *)s1 + i) == *((unsigned char *)s2 + i)) continue;
    if(*((unsigned char *)s1 + i) > *((unsigned char *)s2 + i)) return 1;
    if(*((unsigned char *)s1 + i) < *((unsigned char *)s2 + i)) return -1;
  }
  return 0;
  //panic("Not implemented");
}

#endif
