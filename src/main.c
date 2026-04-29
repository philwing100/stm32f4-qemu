#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#define UART0_BASE 0x101f1000
volatile unsigned int *uart = (volatile unsigned int *)UART0_BASE;

void uart_putchar(char c) { uart[0] = (unsigned int)c; }
void uart_puts(const char *str) { while (*str) uart_putchar(*str++); }
int putchar(int c) { uart_putchar((char)c); return c; }

static void task_hello(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        uart_puts("Task1: Hello World!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

static void task_rtos(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        uart_puts("Task2: FreeRTOS Running!\r\n");
        vTaskDelay(pdMS_TO_TICKS(1500));
    }
}

int main(void) {
    uart_puts("\n====================================\n");
    uart_puts("  STM32F4 + FreeRTOS on QEMU ARM\n");
    uart_puts("====================================\n\n");
    uart_puts("Starting FreeRTOS scheduler...\n\n");

    xTaskCreate(task_hello, "Hello", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(task_rtos,  "RTOS",  configMINIMAL_STACK_SIZE, NULL, 1, NULL);

    vTaskStartScheduler();

    uart_puts("ERROR: Scheduler failed!\n");
    while (1);
    return 0;
}

void vApplicationIdleHook(void) {}
void vApplicationMallocFailedHook(void) { while (1); }
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) { while (1); }
