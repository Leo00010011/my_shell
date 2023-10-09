#ifndef MY_SHELL_GUARD
#define MY_SHELL_GUARD

#include "structures.h"

typedef struct 
{
    linked_list* arg_list;
    linked_node* current_arg;
    int last_result;

}cmd_state;

void ctrlc_handler();

int built_in(cmd_state* cmd, int count_arg);

int process_atom(cmd_state* cmd,linked_list* args);

int process_cmd(cmd_state* cmd);

#endif