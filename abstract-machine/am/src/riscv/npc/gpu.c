#include <am.h>
#include <npc.h>

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
  int w = W;// TODO: get the correct width
  int h = H;// TODO: get the correct height
  uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
  for (i = 0; i < w * h; i ++) fb[i] = i;
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
  // if(ctl->sync){
    int x = ctl->x, y = ctl->y, w = ctl->w, h = ctl->h;
    uint32_t *fb = (uint32_t *)(uintptr_t)FB_ADDR;
    uint32_t *p = (uint32_t *)ctl->pixels;
    int k = 0;
    for(int i = y; i < y + h; i ++){
      // putch('1');
      for(int j = x; j < x + w; j ++){
        fb[W*i+j] = p[k++];
      }
    }
    outl(SYNC_ADDR, 1);
  // }
  
}

void __am_gpu_status(AM_GPU_STATUS_T *status) {
  status->ready = true;
}
