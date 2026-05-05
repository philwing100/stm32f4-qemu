#include "uart.h"
#include "systick.h"
#include "FreeRTOS.h"
#include "task.h"
#include "command_parser.h"
#include "task_manager.h"

static char line_buffer[256];
static uint16_t line_idx = 0;

void uart_rx_task(void *pvParameters) {
    (void)pvParameters;
    volatile int reached = 1;  // ← add this
    (void)reached;
      uart_puts(">> uart_rx_task alive\r\n");
    while (1) {
        int c = uart_getchar_nb();
        if (c >= 0) {
            if (c == '\n' || c == '\r') {
                line_buffer[line_idx] = '\0';
                // Pass line to handler (via queue or global)
                line_idx = 0;
            } else if (line_idx < 255) {
                line_buffer[line_idx++] = c;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void command_handler_task(void *pvParameters) {
    (void)pvParameters;
        uart_puts(">> cmd_handler alive\r\n");
    Command cmd;
    uart_puts("> ");
    while (1) {
            static int loop_count = 0;
    loop_count++;
    if (loop_count % 10 == 0) {
        uart_puts("CMD looping\r\n");
    }
        if (parse_command(line_buffer, &cmd) == 0) {
            switch (cmd.type) {
                case CMD_TASK_CREATE:
                    task_manager_create(cmd.name, cmd.priority, cmd.stack_size);
                    break;
                case CMD_TASK_LIST:
                    task_manager_list();
                    break;
                case CMD_HELP:
                    uart_puts("=== Commands ===\n");
                    uart_puts("TASK_CREATE <name> <priority>\n");
                    uart_puts("TASK_LIST\nHELP\n");
                    break;
                default:
                    uart_puts("Unknown command\n");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

int main(void) {
    
    uart_init(115200);
    task_manager_init();

  uart_puts("=== STM32F4 FreeRTOS QEMU ===\r\n");
  uart_puts("Starting scheduler...\r\n");

BaseType_t r1 = xTaskCreate(uart_rx_task, "UART_RX", 256, NULL, 2, NULL);
BaseType_t r2 = xTaskCreate(command_handler_task, "CMD_HANDLER", 256, NULL, 3, NULL);

if (r1 != pdPASS) uart_puts("UART_RX create FAILED\r\n");
if (r2 != pdPASS) uart_puts("CMD_HANDLER create FAILED\r\n");
    vTaskStartScheduler();
    return 0;
}
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask; (void)pcTaskName;
    while (1);
}
void vApplicationMallocFailedHook(void) { while (1); }
