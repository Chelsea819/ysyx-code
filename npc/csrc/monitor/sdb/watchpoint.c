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

#include <sdb.h>

#define NR_WP 32

// typedef struct watchpoint {
//   int NO;
//   int times;
//   uint32_t data;
//   char *target;
//   struct watchpoint *next;
//   struct watchpoint *past;

//   /* TODO: Add more members if necessary */

// } WP;

WP *wp_pool = NULL;
WP *free_ = NULL;
WP *head = NULL;
//static int gap = 31;

WP* new_wp(char *args){
  //search if existed the same
  bool success = true;
  WP* p_searchExist = head;
  while(p_searchExist != NULL){
    if(strcmp(p_searchExist->target,args) == 0) {
      return NULL;
    }
    p_searchExist = p_searchExist->next;
  }

  //full
  if(free_==NULL)  Assert(0,"wp_pool full!\n");

  //find ava point
  WP* get_wp = free_;
  free_ = free_->next;
  // while(get_wp != NULL){
  //   if(get_wp->next == NULL){
  //     break;
  //   }
  //   get_wp = get_wp->next;
  // }

  //cut it from free_
  // get_wp->past->next = NULL;
  get_wp->times = 0;
  get_wp->target = (char *)malloc(strlen(args)+1);
  strcpy(get_wp->target,args);
  get_wp->data = expr(args,&success);
  printf("get_wp->data = %d\n",get_wp->data);
  Assert(success,"Make_token fail!");

  //add it to head list
  if(head == NULL){
    head = get_wp;
    get_wp->NO = 0;
    get_wp->next = NULL;
    get_wp->past = NULL;
    // printf("the passing head address:%p\n",head);
  }
  else{
    WP* addSpot = head;
    while(addSpot->next != NULL){
      addSpot = addSpot->next;
    }
    addSpot->next = get_wp;
    get_wp->past = addSpot;
    get_wp->NO = addSpot->NO + 1;
    get_wp->next = NULL;
  }

    // WP *index = head;
    // watchPoints_display();
    // while(index){
    //   printf("\n head \nNO:%d  target:%s   %p   data: %x\n",index->NO,index->target,index->target,index->data);
    //   index = index->next;
    // }
  return get_wp;
}

// WP* get_head(){
//   return head;
// }

void free_wp(WP *wp){
  if(!wp) Assert(0,"Free_wp received NULL!\n");

  //remove it from head
  if(wp->next == NULL && wp->past == NULL) head = NULL;
  else{
    if(wp->past != NULL) wp->past->next = wp->next;
    else head = wp->next;
    if(wp->next != NULL) wp->next->past = wp->past;
  }

  free(wp->target);

  wp->data = 0;
  wp->times = 0;

  //add it to free_
  if(!free_) {
    free_ = wp;
    wp->next = NULL;
    wp->NO = 0;
    wp->past = NULL;
    return;
  }
  
  WP *index = free_;
  while(index != NULL){
    if(index->next == NULL){
      index->next = wp;
      wp->past = index;
      wp->next = NULL;
      wp->NO = index->NO + 1;
      break;
    }
    index = index->next;
  }
  //printf("ADD it to %d in free_",wp->NO);
  return;


}

void init_wp_pool() {
  int i;
  wp_pool = (WP *)malloc(NR_WP * sizeof(WP));
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
    wp_pool[i].past = (i == 0 ? NULL : &wp_pool[i - 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

