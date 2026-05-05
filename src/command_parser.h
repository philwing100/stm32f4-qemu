#ifndef COMMAND_PARSER_H
#define COMMAND_PARSER_H

#include <stdint.h>
#include <string.h>   // strcmp, strncpy
#include <stdio.h>  

#define DEFAULT_TASK_STACK_WORDS  128U

typedef enum {
    CMD_UNKNOWN,
    CMD_TASK_CREATE,
    CMD_TASK_LIST,
    CMD_TASK_SUSPEND,
    CMD_TASK_RESUME,
    CMD_TASK_DELETE,
    CMD_HELP
} CommandType;

typedef struct {
    CommandType type;
    char name[32];
    uint32_t priority;
    uint32_t stack_size;
} Command;

int parse_command(const char *line, Command *cmd);

#endif