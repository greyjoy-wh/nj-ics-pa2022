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
#include "memory/vaddr.h"

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");  //猜测是 先输出nemu字符串然后再读入字符串。

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_END;
  return -1;
}

static int cmd_s(char *args) {
  if(args == NULL){
    cpu_exec(1); 
  } else {
    int times;
    sscanf(args,"%d",&times);
    cpu_exec(times);
  }
  return 0;
}

static int cmd_info(char *args) {
  if(args[0] == 'r'){
    isa_reg_display();
  }  
  
  return 0;
}
//声明该函数



static int cmd_pm(char *args) {
  char *num_c = strtok(args, " ");
  char *paddr_c = strtok(NULL," ");
  paddr_c += 2;//右移动两个。
  int num_m;
  paddr_t paddr_m;
  sscanf(num_c,"%d", &num_m);
  sscanf(paddr_c,"%x",&paddr_m);
  for(int i = 0; i < num_m; i++){
    word_t val = vaddr_read(paddr_m, 4);
    printf("0x%x\t\t0x%8x\n", paddr_m, val);
    paddr_m += 4;
  }

  return 0;
}


static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);//函数指针
} cmd_table [] = {
  { "help", "Display information about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },
  { "si","Continue the execution for a certain time(s)", cmd_s},
  { "info", "print something", cmd_info},
  { "x","print val of vaddr", cmd_pm},
  /* TODO: Add more commands */

};

#define NR_CMD ARRLEN(cmd_table) //长度是 3


static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);//str 是指针 +一个整数后就是移动指针
                                      //strlen 长度是不包括最后一个空字符
                                      //sizeof 是包括的最后一个字符
                                      //str_end是最后一个字符的后面一个类似end() 迭代器。
                              
    /* extract the first token as the command */
    char *cmd = strtok(str, " ");  //从 “ ” 中拆分单词
    if (cmd == NULL) { continue; } //如果拆分出来是NULL那么继续读取

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;         //看指令后面有没有参数
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) { //循环判断输入的cmd 指令是哪一个，然后根据不同的函数指针来运行函数。 
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }  
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}
