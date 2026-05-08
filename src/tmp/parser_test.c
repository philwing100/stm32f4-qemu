#include "uart.h"
#include "systick.h"
#include "command_parser.h"

int main(void){
    Command cmd;
    parse_command("TASK_CREATE worker 2", &cmd);
    printf("%d", cmd.priority);
    return cmd.priority;
}