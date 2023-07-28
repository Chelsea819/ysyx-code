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

#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <memory/vaddr.h>

static int is_batch_mode = false;

typedef struct watchpoint {
  int NO;
  int times;
  uint32_t data;
  char *target;
  struct watchpoint *next;
  struct watchpoint *past;

  /* TODO: Add more members if necessary */

} WP;

void init_regex();
void init_wp_pool();
WP* new_wp(char *args);
WP* get_head();
void free_wp(WP *wp);

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char *rl_gets()
{
  static char *line_read = NULL;

  if (line_read)
  {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read)
  {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args)
{
  /*Simulate how the CPU works.*/
  cpu_exec(-1);
  return 0;
}

uint32_t convert_ten(char *args){
  uint32_t flag = 1;
  uint32_t flag_neg = 0;
  uint32_t n = 0;

  int i = strlen(args) - 1;
  for(  ;i >= 0;i --){
    if(args[i] == 'n'){
      flag_neg += 1;
      continue;
    }
    n += ((int)args[i] - (int)'0') * flag;
    flag = flag * 10;
  }
  while(flag_neg --) n = n * (-1);
  return n;
}

uint32_t convert_16(char *args){
  uint32_t addr = 0;
  int flag = 1;
  for(int i = strlen(args) - 1;i >= 2;i --){
    addr += ((int)args[i] - (int)'0') * flag;
    flag *= 16;
  }
  return addr;
}

static int cmd_si(char *args){
  int n = convert_ten(args);
  cpu_exec(n);
  return 0;
}

static int cmd_q(char *args)
{
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_help(char *args);

static int cmd_w(char *args){
  WP* ret =new_wp(args);
  if(ret==NULL) printf("%s already in wp_pool!\n",args); 
  //printf("RET-TARGET:%s\n",ret->target);
  //printf("RET-TARGET:%p\n",args);
  return 0;
}

static int cmd_pcount(char *args){
  //unsigned int addr = convert_16(args);
  //printf("result = 0x%08x\n",addr);
  bool success = false;
  if(args) success = true;
  printf("\033[105m [ %s ] = [%u] [0x%08x]\033[0m\n",args,expr(args,&success),expr(args,&success));
  return 0;
}

static int cmd_x(char *args){
  char *arg1 = strtok(NULL," ");
  char *arg2 = strtok(NULL," ");
  int len = convert_ten(arg1);
  vaddr_t addr = convert_16(arg2);
  for (int i = 0;i < len;i ++){
    printf("\033[105m 0x%08x: \033[0m \t0x%08x\n",addr + i,vaddr_read(addr + i, 4));
  }
  return 0; 
}

static void watchPoints_display(){
  WP *index = get_head();
  printf("head : %p\n",index);
  printf("head->next : %p\n",index->next);
  printf("Num \tTYpe \tDisp \tEnb \tAddress \t What\n");
  while(index != NULL){
    printf("%d \thw watchpoint \tkeep \ty \t %s\n",index->NO,index->target);
    printf(" \tbreakpoint already hit %d time\n",index->times);
    index = index->next;
  }
  return;
}

static int cmd_info(char *args){
  if (*args == 'r')  isa_reg_display();
  if (*args == 'w')  watchPoints_display();
  return 0;
}

static int cmd_d(char *args){
  WP *index = get_head();
  while(index != NULL){
    if(strcmp(args,index->target) == 0){
      free_wp(index);
      printf("Succeed in removing %s!",args);
      return 0;;
    }
    index = index->next;
  }
  printf("No %s in watchpool!\n",args);
  return 0;
}

static struct
{
  const char *name;
  const char *description;
  int (*handler)(char *);
} cmd_table[] = {
    {"help", "Display information about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    {"si", "Execuate one by one.",cmd_si},
    {"info","Print the state of register(r) or the content of watchpoint(w)",cmd_info},
    {"p","Get the result of EXPR.",cmd_pcount},
    {"x","Scan the memory.",cmd_x},
    {"w","Set a watchpoint.",cmd_w},
    {"d","Delete certain watchpoint.",cmd_d},

    /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args)
{
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL)
  {
    /* no argument given */
    for (i = 0; i < NR_CMD; i++)
    {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else
  {
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(arg, cmd_table[i].name) == 0)
      {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode()
{
  is_batch_mode = true;
}

/* Receive commands from user. */
void sdb_mainloop()
{
  if (is_batch_mode)
  {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL;)
  {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL)
    {
      continue;
    }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end)
    {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i++)
    {
      if (strcmp(cmd, cmd_table[i].name) == 0)
      {
        if (cmd_table[i].handler(args) < 0)
        {
          return;
        }
        break;
      }
    }

    if (i == NR_CMD)
    {
      printf("Unknown command '%s'\n", cmd);
    }
  }
}

void init_sdb()
{
  /* Compile the regular expressions. */
  // 编译正则表达式
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
