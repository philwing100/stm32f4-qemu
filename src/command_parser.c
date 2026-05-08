#include "command_parser.h"
#include "task_manager.h"

void parse_command(const char *line, Command *cmd){
    if(!line || !cmd){
        return;
    }

    //Need to split *line by spaces
    char verb[32], name[32];
    uint32_t priority = 0, stack_size = DEFAULT_TASK_STACK_WORDS;
    sscanf(line, "%31s %31s %lu %lu", verb, name, &priority, &stack_size);

    strncpy(cmd->name, name, sizeof(cmd->name) - 1);

    if (strcmp(verb, "TASK_CREATE") == 0) {
        task_manager_create(name, priority, stack_size);
    } else if (strcmp(verb, "TASK_LIST") == 0) {
        task_manager_list();
    } else if (strcmp(verb, "TASK_SUSPEND") == 0) {
        //todo
    } else if (strcmp(verb, "TASK_RESUME") == 0) {
        //todo
    } else if (strcmp(verb, "TASK_DELETE") == 0) {
        task_manager_delete(name);
    } else if (strcmp(verb, "HELP") == 0) {
        uart_puts("=== Commands ===\r\n");
        uart_puts("TASK_CREATE <name> <priority> [stack]\r\n");
        uart_puts("TASK_LIST\r\n");
        uart_puts("TASK_SUSPEND <name>\r\n");
        uart_puts("TASK_RESUME <name>\r\n");
        uart_puts("TASK_DELETE <name>\r\n");
        uart_puts("HELP\r\n");
    } else {
        uart_puts("Unknown command\r\n");
    }
    uart_puts("> ");
}