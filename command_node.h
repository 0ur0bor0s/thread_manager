
#ifndef MEM_TRACER_COMMAND_NODE_H
#define MEM_TRACER_COMMAND_NODE_H

typedef struct command_struct {
   char* cmd;
   int index;
   struct command_struct* next_cmd_ptr;
} CommandNode;

void CreateCommandNode(CommandNode* this_node, char* cmd, int index, CommandNode* next_cmd);
void InsertCommandAfter(CommandNode* this_node, CommandNode* new_node);
void PrintNode(CommandNode* cmd);

#endif //MEM_TRACER_COMMAND_NODE_H
