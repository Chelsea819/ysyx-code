#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

// 系统屏幕大小
static int screen_width = 0;
static int screen_height = 0;

static const char *keyname[256] __attribute__((used)) = {
  [AM_KEY_NONE] = "NONE",
  AM_KEYS(NAME)
};

size_t serial_write(const void *buf, size_t offset, size_t len) {
  int i = 0; 
  for(i = 0; i < len; i ++){
    putch(*((char*)buf + i));
  }
  return i;
}

// 把事件写入到buf中, 最长写入len字节
// 然后返回写入的实际长度
size_t events_read(void *buf, size_t offset, size_t len) {
  int i = 0;
  AM_INPUT_KEYBRD_T ev = io_read(AM_INPUT_KEYBRD);
  // memset(buf,0,sizeof(buf));
  for(i = 0; i < len - 1 && i < strlen(keyname[ev.keycode]); i ++){
    *((char *)buf + i) = keyname[ev.keycode][i];
  }
  *((char *)buf + i) = '\0';
  printf("buf = %s\n",buf);
  return i;
}


// unhandled!
size_t dispinfo_read(void *buf, size_t offset, size_t len) {
  // int i = 0;
  AM_GPU_CONFIG_T ds = io_read(AM_GPU_CONFIG);
  screen_height = ds.height;
  screen_width = ds.width;
  // printf("width = %d height = %d\n",ds.width,ds.height);
  return sprintf(buf, "WIDTH:%d\nHEIGHT:%d",screen_width,screen_height);
}

// 把buf中的len字节写到屏幕上offset处
// 需要先从offset计算出屏幕上的坐标, 然后调用IOE来进行绘图
size_t fb_write(const void *buf, size_t offset, size_t len) {
  assert(offset + len <= screen_width * screen_height * 4);
  // printf("offset = %d screen_width = %d screen_height = %d offset + len = %d\n",offset,screen_width,screen_height,offset + len);
  int x = (offset / 4) % screen_width;
  int y = (offset / 4 - x) / screen_width;
  int w = screen_width - x * 2;
  int h = len / w;
  // printf("[fb_write] (%d, %d) draw %d * %d offset = %d\n",x,y,w,h,offset);
  io_write(AM_GPU_FBDRAW, x, y, (char *)buf, w, h / 4, true);
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
