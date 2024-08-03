/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NPC is licensed under Mulan PSL v2.
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

//#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include "vaddr.h"

enum {
  TK_NOTYPE = 256,  //space
  TK_LEFT_BRA, TK_RIGHT_BRA, //bracket
  TK_NUM, TK_HEXA, TK_REG,DEREF, //data
  //3优先级 * /
  TK_MUL,TK_DIV, 
  //4优先级 + - 
  TK_ADD,TK_SUB,
  //7优先级 == !=
  TK_EQ, TK_NEQ,
  //11优先级&&
  TK_AND,

  /* TODO: Add more token types */

};

//实现了一个基于正则表达式匹配的词法解析规则定义
static struct rule {

  //用于匹配的正则表达式
  const char *regex;
  //匹配成功后对应的语法标记类型
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"==", TK_EQ},        // equal
  {"\\+", TK_ADD},      // plus
  {"-",TK_SUB},         // sub
  {"\\*",TK_MUL},       // mul
  {"/",TK_DIV},         // division
  {"\\(",TK_LEFT_BRA},    // bracket-left
  {"\\)",TK_RIGHT_BRA},   // bracket-right
  {"^0x[a-z0-9]+",TK_HEXA},
  {"[0-9]+",TK_NUM},    // num
  {"^\\$+[a-z0-9]+",TK_REG},
  {"!=",TK_NEQ},
  {"&&",TK_AND},
};

#define NR_REGEX ARRLEN(rules)

//collect the inner content after compling regex
static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
//这样可以预先编译所有正则模式,提高后续匹配效率
//检查每个正则表达式是否编译成功,避免在匹配时才发现错误
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  //遍历正则规则数组rules[],共有NR_REGEX条规则
  for (i = 0; i < NR_REGEX; i ++) {
    /*第三个参数是匹配选项,这里是REG_EXTENDED表示支持扩展正则语法*/
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      printf("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
      assert(0);
    }
  }
}

// typedef struct token {
//   int type;
//   char str[32];
// } Token;

// static Token tokens[32] __attribute__((used)) = {};
// static int nr_token __attribute__((used))  = 0;

// static bool make_token(char *e) {
//   //current position
//   memset(tokens, 0, sizeof(tokens));
//   int position = 0;
//   int i;
//   //Byte offset from string's start to substring' starting and end
//   regmatch_t pmatch;

//   nr_token = 0;
//   int flag_neg = 0;
//   while (e[position] != '\0') {
    
//     /* Try all rules one by one. */
//     for (i = 0; i < NR_REGEX; i ++) {
//       if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
//         char *substr_start = e + position;
//         int substr_len = pmatch.rm_eo;
//         //printf("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
//             //i, rules[i].regex, position, substr_len, substr_len, substr_start);

//         position += substr_len;

//         /* TODO: Now a new token is recognized with rules[i]. Add codes
//          * to record the token in the array `tokens'. For certain types
//          * of tokens, some extra actions should be performed.
//          */

//         //TK_NOTYPE = 256, TK_EQ, TK_SUB,TK_MUL,
//         //TK_DIV, TK_LEFT_BRA, TK_RIGHT_BRA, TK_NUM

//         switch (rules[i].token_type) {
//           case TK_NOTYPE:
//             break;

//           case TK_SUB:
//             if( (nr_token == 0) 
//             || (nr_token > 0 && 
//               ((tokens[nr_token - 1].type >= TK_MUL && tokens[nr_token - 1].type <= TK_AND) || tokens[nr_token - 1].type == TK_LEFT_BRA)))
//             {
//               printf("Get the negtive num! nr_token = %d",nr_token);
//               flag_neg += 1;
//               break;
//             } 

//           case TK_MUL:
//           case TK_DIV:
//           case TK_ADD:
//           case TK_LEFT_BRA:
//           case TK_RIGHT_BRA:
//           case TK_AND:
//           case TK_NEQ:
//           case TK_EQ:
//             tokens[nr_token ++].type = rules[i].token_type;
//             break;

//           //十进制数（进行了对正负的处理）
//           case TK_NUM: 
//             tokens[nr_token].type = rules[i].token_type;
//             assert(substr_len <= 31);
//             strncpy(tokens[nr_token].str, substr_start, substr_len);
//             while(flag_neg --) strcat(tokens[nr_token].str,"n"); 
//             nr_token++;
//             flag_neg = 0;
//             break;

//           //十六进制数
//           case TK_HEXA:
//             tokens[nr_token].type = rules[i].token_type;
//             assert(substr_len <= 32);
//             strncpy(tokens[nr_token++].str, substr_start, substr_len);
//             break;

//           case TK_REG:
//             tokens[nr_token].type = rules[i].token_type;
//             assert(substr_len <= 32);
//             strncpy(tokens[nr_token++].str, substr_start + 1, substr_len);
//             break;


//           default: TODO();
//         }

//         break;
//       }
//     }

//     if (i == NR_REGEX) {
//       printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
//       return false;
//     }
//   }

//   return true;
// }

// static int op_type = 262;

// static int find_main(int p,int q){
//   //printf("enter main_find p = %d, q = %d\n",p ,q);
//   int flag = 0;
//   int flag_bracket = 0;
//   op_type = 262;
//   for(int i = q ; i > p ; i -- ){
//     if(tokens[i].type == TK_RIGHT_BRA){
//       flag_bracket += 1;
//       continue;
//     }
//     else if(tokens[i].type == TK_LEFT_BRA){
//       flag_bracket -= 1;
//       continue;
//     }
//     if(flag_bracket == 0){
//       if(tokens[i].type == TK_AND) {
//         op_type = tokens[i].type;
//         return i;
//       }
//       if(tokens[i].type > op_type) {
//         op_type = tokens[i].type;
//         flag = i;
//         //printf("current op_type = %d flag = %d\n",op_type,flag);
//       }
//     }
//   }
//   //printf("get main_find op_type = %d flag = %d\n",op_type,flag);
//   return flag;
// }

// bool check_parentheses(int p, int q){
//   //printf("enter check\n");
//   int left = 0;
//   int right = 0;
//   int fake_false = 0;
//   if(tokens[p].type != TK_LEFT_BRA  || tokens[q].type != TK_RIGHT_BRA)      return false;
//   else {
//     for(int i = p; i <= q ; i ++){
//       // if(left == 1 && right == 0 && tokens[i].type == TK_RIGHT_BRA) {
//       //   fake_false = 1;
//       // }
//       if(left == right && left!=0  && i != q ) {
//         fake_false = 1;
//         //printf("fake_false = 1\n");
//       }
//       if(tokens[i].type == TK_RIGHT_BRA) {
//         right += 1;
//         //printf("right = %d  i = %d \n",right,i);
//         if(right > left)  {
//           //printf("right = %d left = %d i = %d \n",right,left,i);
//           Assert(0,"Bad expression--too much right brackets");}
//         }
//       else if(tokens[i].type == TK_LEFT_BRA) {
//         left += 1;
//         //printf("left = %d  i = %d \n",left,i);
//       }

//     }
//     if(left != right ) Assert(0,"Bad expression--too much left brackets,left = %d right = %d\n",left,right);
//   }
//   if(fake_false) return false;
//   return true;
// }   

// uint32_t convert_ten(char *args);
// uint32_t convert_16(char *args);

// uint32_t eval(int p, int q){
//   //int num = 0;
//   uint32_t val1 = 0;
//   uint32_t val2 = 0;
//   int op_type1 = 0;
//   int op = 0;
//   bool success = false;
//   char arr1[5] = {0};
  
//   //int error = 0;
//   //printf("initial p = %d ,q = %d\n",p,q);
//   if (p > q) {
//     /* Bad expression */
//     Assert(0,"Bad expression!");
//   }
//   else if (p == q) {
//     /* Single token.
//      * For now this token should be a number.
//      * Return the value of the number.
//      */
//     switch (tokens[p].type){

//     case TK_NUM: return convert_ten(tokens[p].str);

//     //寄存器 or pc 里的值
//     case TK_REG: 
//       //printf("pc current\n");
//       // char *arr1 = (char *)malloc(3*sizeof(char));
//       // memset(arr1,0,3*sizeof(char));
//       // strncpy(arr1,tokens[p].str,2);

//       // free(arr1);
//       strncpy(arr1,tokens[p].str,2);

//       return 1;
      
//       //return isa_reg_str2val(arr1, &success);

//     //十六进制数 
//     case TK_HEXA: 
//       return convert_16(tokens[p].str);
    
//     default:
//       Assert(0,"Unknown content!\n");
//     }
    
//   }
//   else if (check_parentheses(p, q) == true) {
//     /* The expression is surrounded by a matched pair of parentheses.
//      * If that is the case, just throw away the parentheses.
//      */
//     //printf("check_parentheses(p, q) == true, p = %d , q = %d\n",p,q);
//     return eval(p + 1, q - 1);
//   }
//   else {
//     /* We should do more things here. */
//     //printf("before main finding\n");
//     if(tokens[p].type == DEREF){
//       return vaddr_read(convert_16(tokens[p + 1].str),4);
//     }
//     op = find_main(p,q);
//     op_type1 = op_type;
//     val1 = eval(p, op - 1);
//     //printf("val1 = 0x%08x\n",val1);
//     val2 = eval(op + 1, q);
//     //printf("val2 = 0x%08x\n",val2);
//     //printf("find_main op_type: %d\n",op_type);

//     switch (op_type1) {
//       case TK_ADD: return val1 + val2;
//       case TK_SUB: return val1 - val2;
//       case TK_MUL: return val1 * val2;
//       case TK_DIV: return val1 / val2;
//       case TK_AND: return val1 && val2; 
//       case TK_NEQ: return val1 != val2;
//       case TK_EQ:  return val1 == val2;
//       default: assert(0);
    
//     }
//   }
//   return 0;
// }


// word_t expr(char *e, bool *success) {
//   int i = 0;
//   if (!make_token(e)) {
//     *success = false;
//     return 0;
//   }
//   /* TODO: Insert codes to evaluate the expression. */
//   //TODO(); 

//   for (i = 0; i < 32; i ++) {
//     if(tokens[i].type == 0) {
//       break;
//     }
//     if (tokens[i].type == TK_MUL && 
//     (i == 0 || 
//     tokens[i - 1].type >=263 )
//     ) {
//       tokens[i].type = DEREF;
//     }
//   }
//   return eval(0, i - 1);
// }
