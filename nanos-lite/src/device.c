#include <common.h>

#if defined(MULTIPROGRAM) && !defined(TIME_SHARING)
# define MULTIPROGRAM_YIELD() yield()
#else
# define MULTIPROGRAM_YIELD()
#endif

#define NAME(key) \
  [AM_KEY_##key] = #key,

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
  int x = offset % screen_width;
  int y = (offset - x) / screen_height;
  int w = screen_width - x * 2;
  int h = screen_height - y * 2;
  io_write(AM_GPU_FBDRAW, x, y, (uint32_t *)buf, w, h, true);
  return 0;
}

void init_device() {
  Log("Initializing devices...");
  ioe_init();
}
