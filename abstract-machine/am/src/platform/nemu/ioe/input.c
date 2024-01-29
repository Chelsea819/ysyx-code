#include <am.h>
#include <nemu.h>
#include <stdio.h>
#define KEYDOWN_MASK 0x8000

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  kbd->keycode = (inl(KBD_ADDR)) & KEYDOWN_MASK;
  kbd->keydown = (kbd->keycode != 0);
  if(kbd->keycode != 0){
    printf("kbd->keycode = %d\n",kbd->keycode);
    printf("inl(KBD_ADDR) = %d\n",inl(KBD_ADDR));
    printf("(inl(KBD_ADDR)) & KEYDOWN_MASK = %d\n",(inl(KBD_ADDR)) & KEYDOWN_MASK);
    printf("KEYDOWN_MASK = %d\n",KEYDOWN_MASK);
  }
}
