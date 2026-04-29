#include <stdint.h>
#include <string.h>
#include "FreeRTOS.h"

extern int main(void);
extern uint32_t __data_load, __data_start, __data_end;
extern uint32_t __bss_start, __bss_end;

void Default_Handler(void) { while (1); }
void HardFault_Handler(void) { while (1); }
void MemManage_Handler(void) { while (1); }
void BusFault_Handler(void) { while (1); }
void UsageFault_Handler(void) { while (1); }
void SVC_Handler(void) { asm("bx lr"); }
void PendSV_Handler(void);
void SysTick_Handler(void);

__attribute__((section(".isr_vector")))
void (*const g_pfnVectors[])(void) = {
    (void (*)(void))(0x20030000), (void (*)(void))Reset_Handler,
    Default_Handler, HardFault_Handler, MemManage_Handler,
    BusFault_Handler, UsageFault_Handler, Default_Handler,
    Default_Handler, Default_Handler, Default_Handler,
    SVC_Handler, Default_Handler, Default_Handler,
    PendSV_Handler, SysTick_Handler,
};

__attribute__((naked))
void Reset_Handler(void) {
    uint32_t *src = &__data_load;
    uint32_t *dst = &__data_start;
    while (dst < &__data_end) *dst++ = *src++;
    dst = &__bss_start;
    while (dst < &__bss_end) *dst++ = 0;
    main();
    while (1);
}
