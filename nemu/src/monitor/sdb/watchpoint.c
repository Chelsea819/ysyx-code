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
  uint32_t data;
  char *target;
  struct watchpoint *next;
  struct watchpoint *past;

  /* TODO: Add more members if necessary */

} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;
//static int gap = 31;

WP* new_wp(char *args){
  //search if existed the same
  bool success = false;
  WP* p_searchExist = head;
  while(p_searchExist != NULL){
    //printf("%s %s\n",args,p_searchExist->target); 
    if(strcmp(p_searchExist->target,args) == 0) {
      printf("strcmp == 0\n %d %s %s\n",p_searchExist->NO,args,p_searchExist->target); 
      return NULL;
    }
    p_searchExist = p_searchExist->next;
  }

  //full
  if(free_==NULL)  Assert(0,"wp_pool full!\n");

  //find ava point
  WP* get_wp = free_;
  while(get_wp != NULL){
    if(get_wp->next == NULL){
      break;
    }
    get_wp = get_wp->next;
  }
  printf("Succeed in finding an available wp %d\n",get_wp->NO);

  //cut it from free_
  get_wp->past->next = NULL;
  get_wp->next = NULL;
  get_wp->target = malloc(strlen(args)+1);
  strcpy(get_wp->target,args);
  get_wp->data = expr(args,&success);

  //add it to head list
  if(head == NULL){
    head = get_wp;
    get_wp->NO = 0;
    get_wp->past = NULL;
  }
  else{
    WP* addSpot = head;
    while(addSpot != NULL){
      if(addSpot->next == NULL){
        break;
      }
      addSpot = addSpot->next;
    }
    addSpot->next = get_wp;
    get_wp->past = addSpot;
    get_wp->NO = addSpot->NO + 1;
  }
  printf("Succeed in add new wp to %d \n",get_wp->NO);
  printf("%s %s\n",get_wp->target,args);

    WP *index = head;
    while(index){
      printf("\n head \nNO:%d  target:%s   %p\n",index->NO,index->target,index->target);
      index = index->next;
    }

  return get_wp;
}


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
    wp_pool[i].past = (i == 0 ? NULL : &wp_pool[i - 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

