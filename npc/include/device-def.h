#ifndef _NPC__DEVICE_H_
#define _NPC__DEVICE_H_

#define CONFIG_RTC_MMIO 0xa0000048
#define CONFIG_SERIAL_MMIO 0xa00003f8
// #define CONFIG_HAS_PORT_IO 0xa1000000
#define CONFIG_VGA_CTL_MMIO 0xa0000100
#define CONFIG_FB_ADDR 0xa1000000
#define CONFIG_VGA_SHOW_SCREEN 1    //"Enable SDL SCREEN"


#define CONFIG_HAS_SERIAL 1 //"Enable serial"
#define CONFIG_HAS_TIMER 1  //"Enable timer"
#define CONFIG_HAS_VGA 1  //"Enable VGA"
#define CONFIG_VGA_SIZE_800x600 1 //"Screen Size 800 x 600"
#endif