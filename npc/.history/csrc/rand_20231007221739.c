#include "common.h"
#include <time.h>

void init_rand() {
  srand(MUXDEF(CONFIG_TARGET_AM, 0, time(0)));
  //如果定义了CONFIG_TARGET_AM,则返回0,否则返回time(0)的结果
  //time(0)会返回当前的时间戳,不同的时间戳可以产生不同的随机数序列
  //if定义了CONFIG_TARGET_AM,则用0作为随机数种子;否则使用当前时间作为种子
  //这样可以根据不同平台设置不同的随机数种子
}