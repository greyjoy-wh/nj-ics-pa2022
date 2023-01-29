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

void test() {
   FILE *file = fopen("/home/wh/ics2022/nemu/tools/gen-expr/input","r");
  char all[65536];
  if(!file) {
        printf("File opening failed");
    }
    while(fgets(all,65536,file) != NULL) {
   
        
        //assert(0);
        char *res = strtok(all, " ");
        char *cal = strtok(NULL, "\n");
        // printf("%c\n",cal[]);
        // printf("%d\n",(int)strlen(cal));
        int int_res = atoi(res);
        int get_int = expr(cal, NULL);
        
        if(get_int !=int_res){
          printf("false\n");
        }
    }
    printf("success!\n");
    return;
};

int main(int argc, char *argv[]) {

  /* Initialize the monitor. */
#ifdef CONFIG_TARGET_AM
  am_init_monitor();
#else
  init_monitor(argc, argv);
   test();
#endif
  /* Start engine. */
  engine_start();

  return is_exit_status_bad();
}
