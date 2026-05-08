#include "uart.h"
#include "systick.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "command_parser.h"
#include "task_manager.h"
#include "stm32f4_regs.h"

#define LINE_MAX 128

typedef struct {
    char text[LINE_MAX];
} Line;

static QueueHandle_t line_queue;

void uart_rx_task(void *pvParameters) {
    (void)pvParameters;
    uart_puts(">> uart_rx_task alive\r\n");
    uart_puts("> ");

    Line line;
    uint16_t idx = 0;

    while (1) {
        int c = uart_getchar_nb();
        if (c < 0) {
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (c == '\r' || c == '\n') {
            uart_puts("\r\n");
            line.text[idx] = '\0';
            if (idx > 0) {
                xQueueSend(line_queue, &line, 0);
            } else {
                uart_puts("> ");
            }
            idx = 0;
        } else if (c == '\b' || c == 0x7F) {
            if (idx > 0) {
                idx--;
                uart_puts("\b \b");
            }
        } else if (idx < LINE_MAX - 1) {
            uart_putchar((char)c);
            line.text[idx++] = (char)c;
        }
    }
}

void command_handler_task(void *pvParameters) {
    (void)pvParameters;
    uart_puts(">> cmd_handler alive\r\n");

    Line line;
    Command cmd;
    while (1) {
        if (xQueueReceive(line_queue, &line, portMAX_DELAY) != pdTRUE) {
            continue;
        }
        parse_command(line.text, &cmd);
    }
}

int main(void) {
    uart_init(115200);
    task_manager_init();

    uart_puts("=== STM32F4 FreeRTOS QEMU ===\r\n");
    uart_puts("Starting scheduler...\r\n");

    line_queue = xQueueCreate(4, sizeof(Line));

    xTaskCreate(uart_rx_task, "UART_RX", 256, NULL, 2, NULL);
    xTaskCreate(command_handler_task, "CMD_HANDLER", 512, NULL, 2, NULL);
    vTaskStartScheduler();
    return 0;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    (void)xTask;
    uart_puts("OVERFLOW: ");
    uart_puts(pcTaskName);
    uart_puts("\r\n");
    while (1);
}
void vApplicationMallocFailedHook(void) { while (1); }
