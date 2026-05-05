#include "command_parser.h"

int parse_command(const char *line, Command *cmd){
    if(!line || !cmd){
        return -1;
    }

    //Need to split *line by spaces
    char verb[32], name[32];
    uint32_t priority = 0, stack_size = DEFAULT_TASK_STACK_WORDS;
    sscanf(line, "%31s %31s %lu %lu", verb, name, &priority, &stack_size);

    strncpy(cmd->name, name, sizeof(cmd->name) - 1);
    cmd->priority = priority;
    cmd->stack_size = stack_size;

    if (strcmp(verb, "TASK_CREATE") == 0) {
    cmd->type = CMD_TASK_CREATE;
    } else if (strcmp(verb, "TASK_LIST") == 0) {
        cmd->type = CMD_TASK_LIST;
    } else if (strcmp(verb, "TASK_SUSPEND") == 0) {
        cmd->type = CMD_TASK_SUSPEND;
    } else if (strcmp(verb, "TASK_RESUME") == 0) {
        cmd->type = CMD_TASK_RESUME;
    } else if (strcmp(verb, "TASK_DELETE") == 0) {
        cmd->type = CMD_TASK_DELETE;
    } else if (strcmp(verb, "HELP") == 0) {
        cmd->type = CMD_HELP;
    } else {
        cmd->type = CMD_UNKNOWN;
        return -1;
    }

    return 0;

}