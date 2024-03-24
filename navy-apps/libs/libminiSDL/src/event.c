#include <NDL.h>
#include <SDL.h>
#include <assert.h>
#include <string.h>

#define keyname(k) #k,

// static SDL_Event* event_queue = NULL;

static const char *keyname[] = {
  "NONE",
  _KEYS(keyname)
};

int SDL_PushEvent(SDL_Event *ev) {
  assert("TODO!");
  return 0;
}

int SDL_PollEvent(SDL_Event *ev) {
  assert("TODO!");
  return 0;
}

int SDL_WaitEvent(SDL_Event *event) {
  char buf[32];
  while(NDL_PollEvent(buf, sizeof(buf)) == 0);
  if(event == NULL){
    return 0;
  }
  size_t i = 0;
  while (keyname[i] != NULL){
    if(strcmp(keyname[i], buf) == 0)
      break;
    i ++;
  }
  if(keyname[i] == NULL) return 0;
  
  event->key.type = SDL_KEYDOWN;
  event->key.keysym.sym = i;
  return 1;
}

int SDL_PeepEvents(SDL_Event *ev, int numevents, int action, uint32_t mask) {
  assert("TODO!");
  return 0;
}

uint8_t* SDL_GetKeyState(int *numkeys) {
  assert("TODO!");
  return NULL;
}
