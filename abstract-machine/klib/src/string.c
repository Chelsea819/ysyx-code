#include <klib.h>
#include <klib-macros.h>
#include <stdint.h>
#include <stdio.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

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
  printf("---memset begin [size:%d]---\n",n);
  unsigned char *sp = (unsigned char *)s;
  unsigned char val = (unsigned char)c;
  for(int i = 0; i < n; i++){
    *(sp + i) = val;
  }
  printf("---memset end---\n");
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
