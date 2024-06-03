#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <sys/time.h>

static int event_fd;
static int fb_fd;
static int dpinfo_fd;
static int evtdev = -1;
static int fbdev = -1;
static int screen_w = 0, screen_h = 0;  // 画布尺寸
static int sw = 0, sh = 0;  // 屏幕尺寸
static int screenX = 0, screenY = 0;  // 画布起始位置坐标
struct timeval start;

int open(const char *path, int flags, ...);
ssize_t read(int fd, void *buf, size_t count);
int n = 0;

// 以毫秒为单位返回系统时间
uint32_t NDL_GetTicks() {
  struct timeval tv;
  assert(gettimeofday(&tv, NULL) == 0);
  if((tv.tv_usec - start.tv_usec) / 500000 < n ++) printf("tv.tv_usec = %d\nstart.tv_usec = %d\n",tv.tv_usec,start.tv_usec);
  return tv.tv_usec - start.tv_usec;
}

// 在NDL中实现NDL_PollEvent(), 从/dev/events中读出事件并写入到buf中
int NDL_PollEvent(char *buf, int len) {
  read(event_fd, buf, len);
  // printf("buf = %s len = %d\n",buf,len);
  if(strcmp(buf,"NONE") == 0)
    return 0;
  else 
    return 1;
}

// 打开一张 (*w) X (*h)的画布
// 如果*h和*w均为0,则将系统全屏幕作为画布，并将*w和*h分别设为系统屏幕的大小
void NDL_OpenCanvas(int *w, int *h) {
  if (getenv("NWM_APP")) {
    int fbctl = 4;
    fbdev = 5;
    screen_w = *w; screen_h = *h;
    char buf[64];
    int len = sprintf(buf, "%d %d", screen_w, screen_h);
    // let NWM resize the window and create the frame buffer
    assert(write(fbctl, buf, len) != -1);
    while (1) {
      // 3 = evtdev
      int nread = read(3, buf, sizeof(buf) - 1);
      if (nread <= 0) continue;
      buf[nread] = '\0';
      if (strcmp(buf, "mmap ok") == 0) break;
    }
    close(fbctl);
  }
  // 将系统全屏幕作为画布，并将*w和*h分别设为系统屏幕的大小
  char buf[64];
  assert(read(dpinfo_fd, buf, sizeof(buf)) != -1);
  // printf("buf = [%s]\n",buf);
  sscanf(buf, "WIDTH:%d\nHEIGHT:%d", &sw, &sh);
  assert(*w <= sw && *h <= sh);
  // 读出系统全屏幕大小
  if(*w == 0 && *h == 0){
    *w = sw;
    *h = sh;
  }
  screen_w = *w; 
  screen_h = *h;

  // 求出画布起始坐标
  screenX = (sw - screen_w) / 2;
  screenY = (sh - screen_h) / 2;

  // 设置到画布起始坐标
  printf("the size of painting area, width[%d] height[%d]\n",screen_w,screen_h);
  printf("(%d %d)",screenX,screenY);
  lseek(fb_fd, sw * screenY + screenX, SEEK_SET);

}

// void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
//   // printf("(%d, %d) draw %d*%d\n",x,y,w,h);
//   // printf("sw * (y + screenY) + x + screenX = %d [0x%08x]\n",sw * (y + screenY) + x + screenX, sw * (y + screenY) + x + screenX);
//   // h--line write
//   for(int i = 0; i < h; i ++){
//     for(int j = 0; j < h; j ++){
//       for()
//     }


//     for (volatile int j = 0; j < 100000; j++) ;
//   }
// }


// 向画布`(x, y)`坐标处绘制`w*h`的矩形图像, 并将该绘制区域同步到屏幕上
// 图像像素按行优先方式存储在`pixels`中, 每个像素用32位整数以`00RRGGBB`的方式描述颜色
void NDL_DrawRect(uint32_t *pixels, int x, int y, int w, int h) {
  // printf("(%d, %d) draw %d*%d\n",x,y,w,h);
  // printf("sw * (y + screenY) + x + screenX = %d [0x%08x]\n",sw * (y + screenY) + x + screenX, sw * (y + screenY) + x + screenX);
  // h--line write
  assert(pixels);
  // assert(0);
  for(int i = 0; i < h; i ++){
    // int x_i = (screenX + x + (screenY + y + i)) % sw;
    // int y_i = (screenX + x + (screenY + y + i) - x_i) / sw + 1;
    // printf("(x, y) : (%d, %d)", x_i,y_i);
    // printf("(x, y) : (%d, %d)\n", screenX + x,screenY + y + i);
    // printf("(screenX + x + (screenY + y + i) * sw) %d\n",(screenX + x + (screenY + y + i) * sw));
    lseek(fb_fd, ((screenY + y + i) * sw + screenX + x) * 4, SEEK_SET);
    assert(write(fb_fd, pixels + w * i, w * 4) != -1);
    // for (volatile int j = 0; j < 10000000; j++) ;
  }
}

void NDL_OpenAudio(int freq, int channels, int samples) {
}

void NDL_CloseAudio() {
}

int NDL_PlayAudio(void *buf, int len) {
  return 0;
}

int NDL_QueryAudio() {
  return 0;
}

int NDL_Init(uint32_t flags) {
  if (getenv("NWM_APP")) {
    evtdev = 3;
  }
  assert(gettimeofday(&start, NULL) == 0);
  event_fd = open("/dev/event", 0);
  dpinfo_fd = open("/proc/dispinfo", 0);
  fb_fd = open("/dev/fb", 0);
  return 0;
}

void NDL_Quit() {
  close(event_fd);
  close(dpinfo_fd);
}
