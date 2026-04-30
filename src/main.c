#include "uart.h"
#include "systick.h"

int main(void) {
    uart_init(115200);
    uart_puts("=== STM32F4 Bare Metal Boot ===\r\n");
    uart_puts("UART initialized at 115200 baud\r\n");

    systick_init(168000000U);
    uart_puts("SysTick timer running (1ms ticks)\r\n");

    for (int i = 0; i < 10; ++i) {
        delay_ms(1000);
        uart_puts("Tick...\r\n");
    }

    while (1) { }
    return 0;
}
