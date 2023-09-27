/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <common.h>
#ifndef CONFIG_TARGET_AM
#include <time.h>
#endif

void init_rand() {
  srand(MUXDEF(CONFIG_TARGET_AM, 0, time(0)));
  //如果定义了CONFIG_TARGET_AM,则返回0,否则返回time(0)的结果
  //time(0)会返回当前的时间戳,不同的时间戳可以产生不同的随机数序列
  //if定义了CONFIG_TARGET_AM,则用0作为随机数种子;否则使用当前时间作为种子
  //这样可以根据不同平台设置不同的随机数种子
}
