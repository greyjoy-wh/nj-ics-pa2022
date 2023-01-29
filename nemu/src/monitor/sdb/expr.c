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

#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h> //正则表达式的内容

enum {
  TK_NOTYPE = 256, TK_AND = 257, TK_miner = 258, TK_MUL= 259, TK_DIVI = 260,TK_left= 261,TK_right= 262, NUM= 263, TK_EQ= 264


  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces 这里加号的意思是后面右其他的字符
  {"\\+", TK_AND},         // plus
  {"-", TK_miner},
  {"\\*", TK_MUL},
  {"/", TK_DIVI},
  {"\\(", TK_left},
  {"\\)", TK_right},
  {"\\b[0-9]+\\b", NUM},
  {"==", TK_EQ},        // equal
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */

void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);//
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type;
  char str[32];
} Token;

static Token tokens[10000] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      // 
    // if(position == 4) {
    //       printf("%c",e[4]);
    //       assert(0);
    //     }
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0  && pmatch.rm_so == 0) {
        
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;
        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);
        if(rules[i].token_type != TK_NOTYPE){
          Token temp = {rules[i].token_type};
            tokens[nr_token] = temp;
          if(rules[i].token_type == NUM){
            if(substr_len > 32){
              printf("输入的数字不符合规则");
            }
            strncpy(tokens[nr_token].str, substr_start, substr_len);
          }
        }
        nr_token++;
        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        // switch (rules[i].token_type) {
        //   default: TODO();
        // }

        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}

//stack 实现
typedef Token STDataType;
typedef struct Stack
{
	STDataType* _a;
	int _top;//栈顶下标
	int _capacity;//栈的容量
}Stack;

//初始化和销毁
void StackInit(Stack* pst)
{
	assert(pst);//判断指针的有效性
	pst->_a =malloc(sizeof(STDataType) * 4);
	pst->_top = 0;
	pst->_capacity = 4;
}
void StackDestory(Stack* pst)
{
	assert(pst);//判断指针的有效性
	free(pst->_a);
	pst->_a = NULL;
	pst->_top = pst->_capacity = 0;
}
//入栈
void StackPush(Stack* pst, STDataType x)
{
	assert(pst);//判断指针的有效性
	//空间不够则增容
	if (pst->_top == pst->_capacity)
	{
		pst->_capacity *= 2;//每次增容空间的大小为上一次的两倍
		STDataType* tmp = (STDataType*)realloc(pst->_a, sizeof(STDataType) * pst->_capacity);
		if (tmp == NULL)//判断是否内存申请成功
		{
			printf("内存不足!\n");
			exit(-1);
		}
		else
		{
			pst->_a = tmp;
		}
	}
	pst->_a[pst->_top] = x;
	pst->_top++;

}
//出栈
void StackPop(Stack* pst)
{
	assert(pst);//判断指针的有效性
	assert(pst->_top > 0);
	--pst->_top;
}
//获取数据的个数
int StackSize(Stack* pst)
{
	assert(pst);//判断指针的有效性
	return pst->_top;
}
//返回1是空，返回0是非空
int StackEmpty(Stack* pst)
{
	assert(pst);//判断指针的有效性
	return pst->_top == 0 ? 1 : 0;
	//return !pst->_top;
}
//获取栈顶的数据
STDataType StackTop(Stack* pst)
{
	assert(pst);//判断指针的有效性
	assert(pst->_top > 0);
	return pst->_a[pst->_top - 1];
}

//

bool check_parentheses(int p, int q){
  // printf("%d %d\n",tokens[p].type,tokens[q].type);
  if(tokens[p].type != TK_left || tokens[q].type != TK_right) {
  
    return false;
  }
  
  Stack s;
  StackInit(&s);
  // s.push(tokens[p]);
  StackPush(&s, tokens[p]);
  
  for(int i = p + 1; i <= q; i++){
    if(tokens[i].type == TK_left){
      // s.push(tokens[i]);
      StackPush(&s, tokens[p]);

    }
    if(tokens[i].type == TK_right){
      //if(s.top().type != TK_left){
      if(StackTop(&s).type != TK_left){
        printf("你有地方写错了");
        assert(0);
        return false;
      }
      //s.pop();
      
      StackPop(&s);
    }
    //if(s.empty() && i!=q){
    if(StackEmpty(&s) && (i!=q)){  
      return false;
    }
  }
  if(StackEmpty(&s)){
    return true;
  }else{
    printf("你有地方写错了");
        assert(0);
        return false;
  }
}

int eval(int , int );

word_t expr(char *e, bool *success) {
     
  if (!make_token(e)) {
      //  assert(0);
    *success = false;
    return 0;
  }
   
  /* TODO: Insert codes to evaluate the expression. */
  // printf("%c\n", e[nr_token]);

  int ans = eval(0,nr_token-1);


  return (uint32_t)ans;
}

//实现查主符号的方法。
int where_main(int p, int q){
  int res_plus = p;
  int res_multi = p;
  Stack s;
  StackInit(&s);
  for(int i = p; i<=q; i++){
    
    if(tokens[i].type == TK_left){
      //s.push(tokens[i]);
      StackPush(&s, tokens[i]);

    }
    if(tokens[i].type == TK_right){
      //s.pop();
      StackPop(&s);

    }
    //if(!s.empty()){
    if(!StackEmpty(&s)){
      continue;
    }
    if(tokens[i].type == TK_MUL || tokens[i].type == TK_DIVI){
      res_multi = i;
    }
    if(tokens[i].type == TK_AND || tokens[i].type == TK_miner){
      res_plus = i;
    }
  }
  return res_plus == p ? res_multi : res_plus;
}

int eval(int p, int q) {
    if (p > q) {
      printf("你有地方写错了");
      // assert(0);
      return 0;
  }
  else if (p == q) {
    return atoi(tokens[p].str);
  }
  else if (check_parentheses(p, q) == true) {
    /* The expression is surrounded by a matched pair of parentheses.
     * If that is the case, just throw away the parentheses.
     */
    //  assert(0);
    return eval(p + 1, q - 1);
  }
  else {
   int op = where_main(p, q);
   int val1 = eval(p, op - 1);
   int val2 = eval(op + 1, q);

    switch (tokens[op].type) {
      case TK_AND: return val1 + val2;
      case TK_miner: return val1 - val2;
      case TK_MUL: return val1 * val2;
      case TK_DIVI: return val1 / val2;
      default: assert(0);
    }
  }
  return 0;
}