#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "command_node.h"

// create command node
void CreateCommandNode(CommandNode* this_node, char* cmd, int index, CommandNode* next_cmd) {
    this_node->cmd = (char*) malloc(strlen(cmd)+1);
    strcpy(this_node->cmd, cmd);
    this_node->index = index;
    this_node->next_cmd_ptr = next_cmd;
}

// insert command into linked list
void InsertCommandAfter(CommandNode* this_node, CommandNode* new_node) {
    CommandNode* tmp = NULL;

    tmp = this_node->next_cmd_ptr;
    this_node->next_cmd_ptr = new_node;
    new_node->next_cmd_ptr = tmp;
}

// recursive print node function
void PrintNode(CommandNode* cmd) {
    printf("Node index: %d, contains: %s\n", cmd->index, cmd->cmd);

    if (cmd->next_cmd_ptr == NULL) {
        return;
    }

    PrintNode(cmd->next_cmd_ptr);
}


