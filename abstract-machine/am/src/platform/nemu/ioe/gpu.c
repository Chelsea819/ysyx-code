#include <am.h>
#include <nemu.h>

#ifdef MODE_800x600
# define W    800
# define H    600
#else
# define W    400
# define H    300
#endif

#define SYNC_ADDR (VGACTL_ADDR + 4)

void __am_gpu_init() {
  int i;
  // int w = io_read(AM_GPU_CONFIG).width;  // TODO: get the correct width
  // int h = io_read(AM_GPU_CONFIG).height;  // TODO: get the correct height
  int w = W;
  int h = H;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i + 10;
  outl(SYNC_ADDR, 1);
}

void __am_gpu_config(AM_GPU_CONFIG_T *cfg) {
  *cfg = (AM_GPU_CONFIG_T) {
    .present = true, .has_accel = false,
    .width = W, .height = H,
    .vmemsz = 0
  };
}

void __am_gpu_fbdraw(AM_GPU_FBDRAW_T *ctl) { 
  int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  uint32_t *p = (uint32_t *)ctl->pixels;
  // for (int i = 0; i < w * h; i ++) fb[i] = p[i];
  int k = 0;
  for(int i = x; i < x + w; i ++){
    for(int j = y; j < y + h; j ++){
        fb[W*y+x] = p[k++];
    }
  }
  outl(SYNC_ADDR, 1);
  // if (ctl->sync) {
  //   uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  //   uint32_t *p = (uint32_t *)ctl->pixels;
  //   for (int i = 0; i < ctl->w * ctl->h; i ++) fb[i] = p[i];
  //   outl(SYNC_ADDR, 1);
  // }  
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
