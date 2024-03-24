#include <NDL.h>
#include <assert.h>

int SDL_Init(uint32_t flags) {
  return NDL_Init(flags);
}

void SDL_Quit() {
  assert("TODO!");
  NDL_Quit();
}

char *SDL_GetError() {
  assert("TODO!");
  return "Navy does not support SDL_GetError()";
}

int SDL_SetError(const char* fmt, ...) {
  assert("TODO!");
  return -1;
}

int SDL_ShowCursor(int toggle) {
  assert("TODO!");
  return 0;
}

void SDL_WM_SetCaption(const char *title, const char *icon) {
  assert("TODO!");
}

