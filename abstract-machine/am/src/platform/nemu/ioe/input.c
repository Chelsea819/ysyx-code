#include <am.h>
#include <nemu.h>

// 0000_0000_0000_0000_1000_0000_0000_0000
#define KEYDOWN_MASK 0x8000 

void __am_input_keybrd(AM_INPUT_KEYBRD_T *kbd) {
  uint32_t key = inl(KBD_ADDR);
  kbd->keycode = key & ~KEYDOWN_MASK;
  kbd->keydown = ((key & KEYDOWN_MASK) != 0);
}
