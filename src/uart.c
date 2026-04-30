#include "uart.h"
#include "stm32f4_regs.h"

#define UART_PCLK_HZ 42000000U

void uart_init(uint32_t baud) {
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    // PA2 (TX), PA3 (RX) -> alternate function (MODER = 0b10)
    GPIOA->MODER &= ~((3U << 4) | (3U << 6));
    GPIOA->MODER |=  ((2U << 4) | (2U << 6));

    // AF7 (USART2) on PA2, PA3
    GPIOA->AFR[0] &= ~((0xFU << 8) | (0xFU << 12));
    GPIOA->AFR[0] |=  ((7U   << 8) | (7U   << 12));

    USART2->BRR = UART_PCLK_HZ / baud;
    USART2->CR1 = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void uart_putchar(char c) {
    while (!(USART2->SR & USART_SR_TXE)) { }
    USART2->DR = (uint32_t)c & 0xFFU;
}

void uart_puts(const char *s) {
    while (*s) uart_putchar(*s++);
}

int uart_getchar_blocking(void) {
    while (!(USART2->SR & USART_SR_RXNE)) { }
    return (int)(USART2->DR & 0xFFU);
}
