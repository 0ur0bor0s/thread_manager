#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "command_node.h"

// create command node
void CreateCommandNode(CommandNode* this_node, char* cmd, int index, CommandNode* next_cmd) {
    this_node->data = (char*) malloc(strlen(cmd)+1);
    strcpy(this_node->data, cmd);
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

// push new node to head
void PushCommand(CommandNode* new_node, CommandNode** head_ref) {
    //CommandNode* tmp = NULL;

    //tmp = head_node->next_cmd_ptr;
    //head_node->next_cmd_ptr = new_node;
    //new_node->next_cmd_ptr = tmp;

    new_node->next_cmd_ptr = *head_ref;
    *head_ref = new_node;
}

// Get node based off of index
CommandNode* GetNode(CommandNode* node, int index) {
    CommandNode* tmp = node;
    while (tmp != NULL) {
        if (tmp->index == index) {
            return tmp;
        }
        tmp = tmp->next_cmd_ptr;
    }
    return NULL;
}



// recursive print node function
void PrintNode(CommandNode* cmd) {
    printf("Node index: %d, contains: %s\n", cmd->index, cmd->data);

    if (cmd->next_cmd_ptr == NULL) {
        return;
    }

    PrintNode(cmd->next_cmd_ptr);
}


