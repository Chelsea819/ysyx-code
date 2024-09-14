#include <am.h>
#include <nemu.h>
// #define MODE_800x600
// #ifdef MODE_800x600
// # define W    800
// # define H    600
// #else
// # define W    400
// # define H    300
// #endif

// #define SCREEN_W (MUXDEF(CONFIG_VGA_SIZE_800x600, 800, 400))
// #define SCREEN_H (MUXDEF(CONFIG_VGA_SIZE_800x600, 600, 300))

#define SYNC_ADDR (VGACTL_ADDR + 4)
static int WIDTH = 0;
void __am_gpu_init() {
  // int i;
  // int w = W;// TODO: get the correct width
  // int h = H;// TODO: get the correct height
  // uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  // for (i = 0; i < w * h; i ++) fb[i] = i;
  // outl(SYNCADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  uint32_t size = inl(VGACTL_ADDR + 0);
  WIDTH = (int)(size >> 16);
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = WIDTH, .height = (size & 0xffff),
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) { 
    int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
    if (WIDTH == 0) {
      WIDTH = (int)(inl(VGACTL_ADDR + 0) >> 16);
    }
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    uint32_t *p = (uint32_t *)ctl->pixels;
    int k = 0;
    for(int i = y; i < y + h; i ++){
      for(int j = x; j < x + w; j ++){
        fb[WIDTH * i + j] = p[k++];
      }
    }
    outl(SYNC_ADDR, 1); // 刷新一次 将帧缓冲中的内容同步到屏幕上
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
