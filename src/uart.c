#include "uart.h"
#include "stm32f4_regs.h"

#define UART_PCLK_HZ 42000000U

static volatile uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
static volatile uint16_t uart_rx_head = 0;
static volatile uint16_t uart_rx_tail = 0;
static volatile uint16_t uart_rx_current_size = 0;

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

    USART2->CR1 |= USART_CR1_RXNEIE;
    // Set NVIC priority for USART2_IRQ (usually priority 5+)
    NVIC_SetPriority(USART2_IRQn, 5);
    NVIC_EnableIRQ(USART2_IRQn);
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

void USART2_IRQHandler(void) {
    if (USART2->SR & USART_SR_RXNE) {
        uint8_t c = USART2->DR;
        uart_rx_buffer[uart_rx_head] = c;
        uart_rx_head = (uart_rx_head + 1) % UART_RX_BUFFER_SIZE;
    }
}

int uart_getchar_nb(void) {
    if (uart_rx_head == uart_rx_tail) return -1;  // empty
    uint8_t c = uart_rx_buffer[uart_rx_tail];
    uart_rx_tail = (uart_rx_tail + 1) % UART_RX_BUFFER_SIZE;
    return (int)c;
}
