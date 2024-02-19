#include <stdarg.h>

#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

#define BUF_SIZE 1024*4

// 鐢ㄤ簬鍙嶈浆鎸囧畾瀛楃涓?
static void reverse(char *str, int len) {
  assert(NULL != str);
  assert(0 < len);

  char *end = str + len - 1;
  char tmp;
  while (str < end)
  {
    tmp = *str;
    *str = *end;
    *end = tmp;
    str++;
    end--;
  }
}

// 鏃犵鍙锋暟itoa
static int uitoa(unsigned int num, char *str, int base) {
  assert(NULL != str);
  assert(16 >= base);
  assert(0 < base);

  int i = 0;
  int bit;
  do {
    bit = num % base;
    if (10 <= bit) str[i++] = 'a' + bit -10;
    else str[i++] = '0' + bit;
  } while ((num /= base) > 0);
  str[i] = '\0';
  reverse(str, i);
  
  return i;
}

// 绗﹀彿鏁癷toa
static int itoa(int num, char *str, int base) {
  assert(NULL != str);
  assert(16 >= base);
  assert(0 < base);

  int i = 0;
  int sign = num;
  int bit;
  if (sign < 0) num = -num;
  do {
    bit = num % base;
    if (10 <= bit) str[i++] = 'a' + bit - 10;
    else str[i++] = '0' + bit;
  } while ((num /= base) > 0);
  if (sign <  0) str[i++] = '-';
  str[i] = '\0';
  reverse(str, i);

  return i;
}

static int intlen(int num, int base) {
  assert(16 >= base);
  assert(0 < base);

  int i = 0;
  int sign = num;
  if (sign < 0) num = -num;
  do {
    i++;
  } while ((num /= base) > 0);
  if (sign < 0) i++;
  return i;
}

int offset(char *out, size_t *n, size_t padding) {
  assert(NULL != out);
  assert(NULL != n);

  char *start = out;
  size_t this_n = *n;
  while (0 < padding && 0 < this_n) {
    *out++ = ' ';
    this_n--;
    padding--;
  }
  *n = this_n;
  return out - start;
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  assert(NULL != out);
  assert(NULL != fmt);

  char *start = out;
  n--; // 鐣欎竴涓瓧鑺傜粰瀛楃涓茬粨灏剧殑'\0'
  int val;
  size_t off_len;
  const char *str;

  while ('\0' != *fmt && 0 < n) {
    if ('%' == *fmt) {
      fmt++; // 璺宠繃'%'
      
      // 澶勭悊鏍煎紡鍖栨爣蹇?
      int leftJustify = 0;
      if ('-' == *fmt) {
        leftJustify = 1;
        fmt++;
      }

      // 澶勭悊瀛楁瀹藉害
      int width = 0;
      while ('0' <= *fmt && '9' >= *fmt) {
        width = width * 10 + (*fmt - '0');
        fmt++;
      }

      // // 澶勭悊鏍煎紡鍖栫被鍨?
      if ('l' == *fmt) {
        fmt++; // FIXME锛氬湪杩欓噷閫夋嫨浜嗚烦杩?l'浣嗘槸涓嶇‘瀹氭槸涓嶆槸姝ｇ‘鐨勫仛娉?
      }

      // 澶勭悊鏍煎紡鍖栧瓧绗︿覆
      switch (*fmt) {
        case '%': {
          *out++ = *fmt++; 
          n--; 
          break;
        }
        case 'd': {
          val = va_arg(ap, int);
          off_len = intlen(val, 10);

          if (0 < width && !leftJustify) out += offset(out, &n, width - off_len);
          out += itoa(val, out ,10);
          n -= off_len;
          if (0 < width && leftJustify) out += offset(out, &n, width - off_len);
          fmt++;

          break;
        }
        case 'x': {
          val = va_arg(ap, int); // FIXME: 杩欎釜stdio涓墍鏈夌殑涓巌nt鏈夊叧鐨勶紝鏈夊彲鑳借秴杩囧彉閲忕殑鑼冨洿绛夋剰澶栨儏鍐碉紝娉ㄦ剰妫€鏌ワ紝鐩墠灏氭病鏈夋兂鍒版湁浠€涔堟柟娉曡В鍐?
          off_len = intlen(val, 16); // FIXME: 鍐欑殑鐪熼浮宸村睅锛屽彲浠ョ洿鎺ュitoa鍋氬鐢?

          if (0 < width && !leftJustify && 0 < width - off_len) out += offset(out, &n, width - off_len);
          out += itoa(val, out, 16);
          n -= off_len;
          if (0 < width && leftJustify && 0 < width - off_len) out += offset(out, &n, width - off_len);
          fmt++;

          break;
        }
        case 'p': {
          *out++ = '0'; 
          n--;
          *out++ = 'x'; 
          n--;

          val = va_arg(ap, int);
          off_len = intlen(val, 16);

          if (0 < width && !leftJustify && 0 < width - off_len) out += offset(out, &n, width - off_len);
          out += uitoa(val, out, 16);
          n -= off_len;
          if (0 < width && leftJustify && 0 < width - off_len) out += offset(out, &n, width - off_len);
          fmt++;

          break;
        }
        case 'c': {
          *out++ = va_arg(ap, int); 
          n--;
          fmt++;
          break;
        }
        case 's': {
          str = va_arg(ap, const char*);
          off_len = strlen(str);
    
          if (0 < width && !leftJustify && 0 < width - off_len) out += offset(out, &n, width - off_len);
          while (*str && 0 < n) {
            *out++ = *str++;
            n--;
          }
          if (0 < width && leftJustify && 0 < width - off_len) out += offset(out, &n, width - off_len);
          fmt++;

          break;
        }
        default: {
          *out++ = *fmt++; 
          n--; 
          break;
        }
      }
    } else { // 闈炴牸寮忓寲瀛楃锛岀洿鎺ュ鍒跺埌缂撳啿鍖?
      *out++ = *fmt++;
      n--;
    }
  }
  *out = '\0';
  return out - start;
}

int printf(const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  char out[BUF_SIZE];
  // DESCRIPTION:鍦ㄩ€欒鎴戝仛浜嗕竴鍊嬪Ε鍗旓紝鎴戜笉鍘诲仛鍒ゆ柗鏄惁鎵撳嵃瀹岀暍涓︿笖鍦╞uf婊跨殑鏅傚€欏埛鏂板啀瀵叆銆?
  // 鎴戦伕鎿囧彧鍏佽ū鍑芥暩鍚慴uf涓鍏UFF_SIZE澶у皬鐨勬暩鎿氾紝涓︿笖鍙厑瑷辨墦鍗伴€欓杭澶氥€傛帯鐢ㄤ簡KISS鍘熷墖
  int length = vsprintf(out, fmt, ap);
  assert(length <= BUF_SIZE);
  for (int i = 0; i < length; i++) {
    putch(out[i]);
  }
  va_end(ap);
  return length;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int length = vsprintf(out, fmt, ap);
  va_end(ap);
  return length;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int length = vsnprintf(out, n, fmt, ap);
  va_end(ap);
  return length;
}

int vsprintf(char *out, const char *fmt, va_list ap) {
  return vsnprintf(out, BUF_SIZE, fmt, ap);
}

#endif