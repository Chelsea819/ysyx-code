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
  //int i = 0;
  //int index = 0;
  WP* p_searchExist = NULL;
  WP* p_avaiWp = NULL;
  WP* p_addToHead = NULL;
  WP* index = NULL;
  
  //print
  index = head;
  while(index){
    printf("\033[105m \n before search NO:%d  target:%s \033[0m \n",index->NO,index->target);
    index = index->next;
  }

  //search if existed the same
  p_searchExist = head;
  while(p_searchExist != NULL){
    //printf("%s %s\n",args,p_searchExist->target); 
    if(strcmp(p_searchExist->target,args) == 0) {
      printf("strcmp == 0\n %d %s %s\n",p_searchExist->NO,args,p_searchExist->target); 
      return NULL;
    }
    p_searchExist = p_searchExist->next;
  }
  //no available wp
  if(!free_) Assert(0,"No available wp\n");

  //print
  index = head;
  while(index){
    printf("\033[105m \n before search ava NO:%d  target:%s \033[0m \n",index->NO,index->target);
    index = index->next;
  }

  //search for the last available wp
  p_avaiWp = free_;
  while(p_avaiWp != NULL){
    if(p_avaiWp->next == NULL) {
      printf("Succeed in finding an available wp %d\n",p_avaiWp->NO);
      break;
    }
    p_avaiWp = p_avaiWp->next;
  }

  //get the last available wp
  WP* get_wp = p_avaiWp;
  WP* past_wp = p_avaiWp - 1;
  if(!get_wp) Assert(0,"Fail in getting available wp!\n");

  //print
  index = head;
  while(index){
    printf("\033[105m \n before cut NO:%d  target:%s \033[0m \n",index->NO,index->target);
    index = index->next;
  }

  //cut
  if(p_avaiWp->NO == 0) { printf("free_ = NULL\n"); free_ = NULL;}
  else past_wp->next = NULL;

  //print
  index = head;
  while(index){
    printf("\033[105m \n before add NO:%d  target:%s \033[0m \n",index->NO,index->target);
    index = index->next;
  }

  //add new wp to head
  p_addToHead = head;
  if(!head) {
    get_wp->NO = 0;
    get_wp->next = NULL;
    get_wp->target = args;
    head = get_wp;
    printf("Succeed in add new wp to head \n");
    printf("%s %s %s\n",head->target,get_wp->target,args);
  }
  else {
    past_wp = NULL;
    while(p_addToHead){
      if(p_addToHead->next == NULL){
        p_addToHead->next = get_wp;
        break;
      }
      past_wp = p_addToHead;
      p_addToHead = p_addToHead->next;
    }
    get_wp->next = NULL;
    get_wp->NO = p_addToHead->NO + 1;
    get_wp-> target = args; 
    printf("Succeed in add new wp to %d \n",get_wp->NO);
    printf("%s %s %s\n",p_addToHead->next->target,get_wp->target,args);
  }
  
  //print
  index = head;
  while(index){
    printf("NO:%d  target:%s\n",index->NO,index->target);
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
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

