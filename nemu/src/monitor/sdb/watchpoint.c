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

#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  int data;
  char *target;
  struct watchpoint *next;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

WP* new_wp(char *args){
  int i = 0;
  int index = 0;
  //no available wp
  for(int k = 0; k < 32; k++){
    printf("1\n");
    if(head[k].next == NULL) break;
    if(strcmp(args,head[k].target) == 0) return NULL;
  }
  if(!free_) Assert(0,"No available wp\n");

  //search for the last available wp
  for(i = 0;i < NR_WP; i ++){
    if(free_[i].next == NULL){
      printf("Succeed in finding an available wp %d\n",i);
      break;
    }
  }

  //get the last available wp
  WP* get_wp = free_ + i;
  if(!get_wp) Assert(0,"Fail in getting available wp!\n");

  //cut
  if(i == 0) { printf("free_ = NULL\n"); free_ = NULL;}
  else free_[i - 1].next = NULL;

  //add new wp to head
  if(!head) {
    head = get_wp;
    get_wp->NO = 0;
  }
  else {
    for(index = 0;index < NR_WP; index ++){
      if(head[index].next == NULL){
        break;
      }
    }
    head[index].next = get_wp;
    get_wp->NO = index;
  }

  get_wp->target = args;

  

  return get_wp;
}
//cong head qudiao
void free_wp(WP *wp){
  if(!wp) Assert(0,"Free_wp received NULL!\n");
  wp->next = NULL;

  if(!free_) {
    free_ = wp;
    return;
  }
  
  int i = 0;
  for(i = 0;i < NR_WP; i ++){
    if(free_[i].next == NULL){
      break;
    }
  }
  free_[i].next = wp;
  memset(free_[i].target,0,strlen(free_[i].target));
  free_[i].NO = i;

  if(i == 0) head = NULL;
  else head[i - 1].next = NULL;

  return;


}

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

