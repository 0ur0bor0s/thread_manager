
#ifndef MEM_TRACER_COMMAND_NODE_H
#define MEM_TRACER_COMMAND_NODE_H

typedef struct command_struct {
   char* data;
   int index;
   struct command_struct* next_cmd_ptr;
} CommandNode;

void CreateCommandNode(CommandNode* this_node, char* cmd, int index, CommandNode* next_cmd);
void InsertCommandAfter(CommandNode* this_node, CommandNode* new_node);
CommandNode* GetNode(CommandNode* node, int index);
void PushCommand(CommandNode* new_node, CommandNode** head_ref);
void PrintNode(CommandNode* cmd);

#endif //MEM_TRACER_COMMAND_NODE_H
