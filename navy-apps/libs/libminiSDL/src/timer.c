#include <NDL.h>
#include <sdl-timer.h>
#include <stdio.h>

SDL_TimerID SDL_AddTimer(uint32_t interval, SDL_NewTimerCallback callback, void *param) {
  panic("TODO!");
  return NULL;
}

int SDL_RemoveTimer(SDL_TimerID id) {
  panic("TODO!");
  return 1;
}

uint32_t SDL_GetTicks() {
  panic("TODO!");
  return 0;
}

void SDL_Delay(uint32_t ms) {
  panic("TODO!");
}