#include "systick.h"
#include "stm32f4_regs.h"

static volatile uint32_t g_ticks;

void systick_init(uint32_t cpu_hz) {
    SYSTICK->LOAD = (cpu_hz / 1000U) - 1U;
    SYSTICK->VAL  = 0;
    SYSTICK->CTRL = SYSTICK_CTRL_CLKSOURCE | SYSTICK_CTRL_TICKINT | SYSTICK_CTRL_ENABLE;
}

void SysTick_Handler(void) {
    ++g_ticks;
}

uint32_t systick_now(void) {
    return g_ticks;
}

void delay_ms(uint32_t ms) {
    uint32_t start = g_ticks;
    while ((g_ticks - start) < ms) { }
}
