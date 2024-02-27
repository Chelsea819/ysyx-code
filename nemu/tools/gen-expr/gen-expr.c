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

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>
#include <assert.h>
#include <math.h>
#include <string.h>
// this should be enough
//将随机生成的表达式输出到缓冲区buf中
static char buf[65536] = {};
static int index_buf = 0;
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
//模板字符串
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static char *code_format2 =
"#include <stdio.h>\n"
"#include <signal.h>\n"
"#include <sys/types.h>\n"
"#include <math.h>\n"
"int flag = 0;"
"int main() { "
"  float result = %s; "
"  if(isnan(result)) result = -1;"
"  printf(\"%%f\", result); "
"  return 0; "
"}";

static int safe_assure(){
  if(index_buf >= 32)
    return 1;
  return 0;
}

uint32_t choose(uint32_t n){
  return rand()%n;
}

char gen_num(){ 
  int num = rand()%10;
  char numArr[]="0123456789";
  buf[index_buf ++] = numArr[num];
  //printf("get %c\n",numArr[num]);
  return 0;
}

uint32_t gen(char type){
  buf[index_buf ++] = type;
  //printf("get %c\n",type);
  return 0;
}

char gen_rand_op(){ 
  int type = rand()%4;
  char numArr[]="+-*/";
  buf[index_buf ++] = numArr[type];
  //printf("get %c\n",numArr[type]);
  return 0;
}

static int gen_rand_expr() {
  uint32_t i = 0;
  i = choose(3);
  if(safe_assure()) return -1;
 
  //printf("choose(3) = %d\n",i);
  switch (i) {
    case 0: {gen_num(); break;}
    case 1: {gen('('); gen_rand_expr(); gen(')'); break;}
    default: {gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;}
  }
  return 0; 
 
}


int main(int argc, char *argv[]) {
  //生成随机数种子
  int seed = time(0);
  srand(seed);
  int loop = 1;
  //将所需生成测试用例的数量存入loop变量
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    //while(result){  
      memset(buf,0,strlen(buf));
      index_buf = 0;

      //count_debug

      if(gen_rand_expr() == -1 || strlen(buf) > 31) {
        i-=1;
        continue;
      }

      //将要算的表达式放入代码中，生成“计算器”
      sprintf(code_buf, code_format, buf);
      sprintf(code_buf, code_format2, buf);
      //printf("%s \n",code_buf);

      FILE *fp2 = fopen("/tmp/.code.c", "w");
      assert(fp2 != NULL);
      fputs(code_buf, fp2);
      fclose(fp2);

      //执行shell命令
      int ret2 = system("gcc /tmp/.code.c -o /tmp/.expr");
      if (ret2 != 0) continue;

      //使用 popen() 打开一个管道,与 /tmp/.expr 进程的输出交互
      //输出将存入fp
      fp2 = popen("/tmp/.expr", "r");
      assert(fp2 != NULL);

      float result2;
      ret2 = fscanf(fp2, "%f", &result2);
      pclose(fp2);

      if(isnan(result2)) { 
        i -= 1;  
        //printf("zero divide!\n");  
        continue;
      }
      if(result2 < 0) { 
        i -= 1; 
        //printf("succeed in catching neg!\n");  
        continue;
      }

      FILE *fp = fopen("/tmp/.code.c", "w");
      assert(fp != NULL);
      fputs(code_buf, fp);
      fclose(fp);

      //执行shell命令
      int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
      if (ret != 0) continue;

      //使用 popen() 打开一个管道,与 /tmp/.expr 进程的输出交互
      //输出将存入fp
      fp = popen("/tmp/.expr", "r");
      assert(fp != NULL);

      int result;
      ret = fscanf(fp, "%d", &result);
      pclose(fp);
      //}
    printf("%u %s\n", result, buf);
  }
  return 0;
}
