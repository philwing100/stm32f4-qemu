#ifndef UART_H
#define UART_H

#include <stdint.h>

void uart_init(uint32_t baud);
void uart_putchar(char c);
void uart_puts(const char *s);
int  uart_getchar_blocking(void);

#define UART_RX_BUFFER_SIZE 256

void USART2_IRQHandler(void);
int uart_getchar_nb(void);

#endif
