#include <am.h>
#include <klib.h>
#include <klib-macros.h>
#include <stdarg.h>

#if !defined(__ISA_NATIVE__) || defined(__NATIVE_USE_KLIB__)

static char HEX[] = "0123456789ABCDEF";

int printf(const char *fmt, ...) {
    va_list args;
    va_start(args,fmt);
    //char *out=(char*)malloc(4000 * sizeof(char));
    char out[128];
    int ret= vsprintf(out, fmt, args);
    putstr(out);
    va_end(args);
    //free(out);
    return ret;
}

int vsprintf(char *out, const char *fmt, va_list ap){
    char buffer[128];
    char *txt;//字符串类型和字符类型
    int num,len;
    int width=0;
    int state=0;
    int i,j;
    for(i=0,j=0;fmt[i]!='\0';i++){
        switch(state){
            case 0://正常复制
                if(fmt[i]!='%'){
                    out[j]=fmt[i];
                    j++;
                }
                else{
                    state=1;
                }
                break;
            case 1://类型匹配
                if (fmt[i] >= '0' && fmt[i] <= '9') {
                    width = width * 10 + (fmt[i] - '0'); // 解析字段宽度
                } 
                else {
                    switch(fmt[i]){
                        case 's':
                            txt = va_arg(ap, char *);
                                len = 0;
                                while (txt[len] != '\0') len++;
                                for (int k = 0; k < len && k < width; k++) {
                                    out[j] = txt[k];
                                    j++;
                                }
                                for (int k = len; k < width; k++) {
                                    out[j] = ' ';
                                    j++;
                                }
                                break;


                        case 'd':
                                num = va_arg(ap, int);
                                if (num == 0) {
                                    out[j] = '0';
                                    j++;
                                    break;
                                }
                                if (num < 0) {
                                    out[j] = '-';
                                    j++;
                                    num = -num;
                                }
                                len = 0;
                                do {
                                    buffer[len++] = HEX[num % 10];
                                    num /= 10;
                                } while (num);
                                while (len < width) {
                                    buffer[len++] = ' ';
                                }
                                while (len > 0) {
                                    out[j] = buffer[--len];
                                    j++;
                                }
                                break;

                        case 'c':
                            out[j] = va_arg(ap, int);
                                j++;
                                for (int k = 1; k < width; k++) {
                                    out[j] = ' ';
                                    j++;
                                }
                                break;
                        case 'l':
                            if (fmt[++i] == 'd') {
                                num = va_arg(ap, long);
                                if (num == 0) {
                                    out[j] = '0';
                                    j++;
                                    break;
                                }
                                if (num < 0) {
                                    out[j] = '-';
                                    j++;
                                    num = -num;
                                }
                                len = 0;
                                do {
                                    buffer[len++] = HEX[num % 10];
                                    num /= 10;
                                } while (num);
                                while (len < width) {
                                    buffer[len++] = ' ';
                                }
                                while (len > 0) {
                                    out[j] = buffer[--len];
                                    j++;
                                }
                            }
                            break;
                        default:
                        putch(fmt[i-1]);
                        putch(fmt[i]);
                        putch(fmt[i+1]);
                        assert(0);
                    }
                }
            state=0;
            width=0;
            break;
        }
    }
    out[j]='\0';
    return j;
}

int sprintf(char *out, const char *fmt, ...) {
  va_list args;
  int i;
  va_start(args,fmt);
  i=vsprintf(out,fmt,args);
  va_end(args);
  return i;
}

int snprintf(char *out, size_t n, const char *fmt, ...) {
  panic("Not implemented");
}

int vsnprintf(char *out, size_t n, const char *fmt, va_list ap) {
  panic("Not implemented");
}

#endif