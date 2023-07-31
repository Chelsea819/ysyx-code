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

void init_monitor(int, char *[]);
void am_init_monitor();
void engine_start();
int is_exit_status_bad();
word_t expr(char *e, bool *success);
uint32_t convert_ten(char *args);

int main(int argc, char *argv[]) {
  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
#endif

  FILE * fp = fopen("~/ysyx-workbench/nemu/tools/gen-expr/build", "r");
  if(fp == NULL){
    Log("Can not open 'input' !");
    return 0;
  }
  char* arr = NULL;
  char* result = NULL;
  char* exp = NULL;
  bool success = false;
  word_t result_exp = 0;
  word_t result_before = 0;
  while(fgets(arr,100,fp) != NULL){

    result = strtok(arr," ");
    exp = strtok(NULL," ");

    if(strlen(exp) >= 30)  continue;

    result_exp = expr(exp,&success);
    result_before = convert_ten(result);

    if(result_exp != result_before) Log("result_exp != result_before");
  }

  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
