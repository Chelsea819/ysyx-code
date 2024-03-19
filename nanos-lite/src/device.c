#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  int i = 0; 
  for(i = 0; i < len; i ++){
    putch(*((char*)buf + i));
  }
    // printf("i = %d\n",i);
  return i;
}

// 把事件写入到buf中, 最长写入len字节
// 然后返回写入的实际长度
size_t events_read(void *buf, size_t offset, size_t len) {
  int i = 0;
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  printf("sizeof(keyname[ev.keycode] = %d\n",sizeof(keyname[ev.keycode]));
  for(i = 0; i < len && i < strlen(keyname[ev.keycode]); i ++){
    *((char *)buf + i) = keyname[io_read(AM_INPUT_KEYBRD).keycode][i];
  }
  *((char *)buf + i) = '\0';
  printf("buf = %s ev.keycode = %d ev.keydown = %d\n",buf ,ev.keycode,(ev.keydown ? 1 : 0));
  
  return i;
}

size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  return 0;
}

size_t fb_write(const void *buf, size_t offset, size_t len) {
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
