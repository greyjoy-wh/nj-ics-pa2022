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
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
int buf_cur = 0;//buf 的偏移量
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

uint32_t chose(uint32_t num ){
    return rand()%num;//生成0-2的随机数
}

int get_wei(int n){
    int count = 0; //函数内部的局部变量一定要初始化 不然就会出现奇怪的错误！！
    while(n) {
        n = n / 10;
        count ++;
    }
    return count;
}

void gen_num(){
 int num = rand()%100 + 1; //生成1-100的数;
  sprintf(buf+buf_cur, "%d", num);
  buf_cur += get_wei(num);
  return;
}

void gen(const char* c) {
    sprintf(buf + buf_cur, "%s", c); 
  buf_cur++;
}

void gen_op(){
  int num = rand()%4 + 1; //生成1-4;
  switch(num){
    case 1: sprintf(buf + buf_cur, "%s", "+"); break;
    case 2: sprintf(buf + buf_cur, "%s", "-"); break;
    case 3: sprintf(buf + buf_cur, "%s", "*"); break;
    default: sprintf(buf + buf_cur, "%s", "/"); break;
  }
  buf_cur++;
  return;
}

static void gen_rand_expr() {
    int n = chose(3);
    switch(n) {
      case 0: gen_num(); break;
      case 1: gen("("); gen_rand_expr(); gen(")"); break;
      default:gen_rand_expr(); gen_op(); gen_rand_expr(); break;
    }
  return;
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    buf_cur = 0;
    gen_rand_expr();
    //printf("%s",buf);
    //buf[]='\0';
    sprintf(code_buf, code_format, buf); //该api就是将code_format中的标志服替换成buf,然后将整个写入到code_buf流中
                                         //用强行拼凑的方式将原来是字符串的源代码 以及 我们生成的字符串代码拼到一起。
    FILE *fp = fopen("/tmp/.code.c", "w");//创建一个文件并且是用来写入东西。
    assert(fp != NULL);
    fputs(code_buf, fp);//将code_buf写入到 上面创建的文件流中。
    fclose(fp);//关闭流

    int ret = system("gcc /tmp/.code.c  -o /tmp/.expr");//执行后面的代码，也就是编译 我们写入文件的检验c代码
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r"); //执行文件，将输出写入流fp中
    assert(fp != NULL);

    int result;
    fscanf(fp, "%d", &result); //从上面的输出流中读取数字，并且写到result中
    if(pclose(fp)!=0){
      continue;
    }//如果没有正常关闭，也就是报错或者警告直接跳过 不输出这个结果。

    printf("%u %s\n", result, buf);//输出 结果 与表达式、
  }
  return 0;
}

// #include <stdint.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <time.h>
// #include <assert.h>
// #include <string.h>

// // this should be enough
// static char buf[65536];
// static int bufPos;
// static char ops[]={'+','-','*','/'};
// static int choose(int mx){
// 	return rand()%mx;
// }
// static inline void gen(char c){
// 	buf[bufPos++]=c;
// }
// static inline void gen_num(){
// 	int len=choose(6);
// 	gen(choose(2)+'1');
// 	while(len--){
// 		gen(choose(10)+'0');
// 	}
// }
// static inline void gen_op(){
// 	gen(ops[choose(sizeof(ops)/sizeof(char))]);
// }
// const int MAXN=10;
// static inline void gen_rand_expr() {
// 	  switch(choose(3)){
// 			case 0: gen_num(); break;
// 			case 1: gen('('); gen_rand_expr(); gen(')'); break;
// 		  default: gen_rand_expr(); gen_op(); gen_rand_expr(); break;
// 		}
// }

// static char code_buf[65536];
// static char *code_format =
// "#include <stdio.h>\n"
// "int main() { "
// "  unsigned result = %s; "
// "  printf(\"%%u\", result); "
// "  return 0; "
// "}";

// int main(int argc, char *argv[]) {
//   int seed = time(0);
//   srand(seed);
//   int loop = 1;
//   if (argc > 1) {
//     sscanf(argv[1], "%d", &loop);
//   }
//   int i;
//   for (i = 0; i < loop; i ++) {
// 		bufPos = 0;
//     gen_rand_expr();
// 		buf[bufPos]='\0';
//     sprintf(code_buf, code_format, buf);

//     FILE *fp = fopen(".code.c", "w");
//     assert(fp != NULL);
//     fputs(code_buf, fp);
//     fclose(fp);

//     int ret = system("gcc .code.c -o .expr");
//     if (ret != 0) continue;

//     fp = popen("./.expr", "r");
//     assert(fp != NULL);

//     int result;
//     fscanf(fp, "%d", &result);
//     pclose(fp);

//     printf("%u %s\n", result, buf);
//   }
//   return 0;
// }