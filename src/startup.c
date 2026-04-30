#include <stdint.h>

extern int main(void);
extern uint32_t __data_load, __data_start, __data_end;
extern uint32_t __bss_start, __bss_end;

void Reset_Handler(void);
void Default_Handler(void) __attribute__((weak));
void NMI_Handler(void)        __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void MemManage_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void BusFault_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void UsageFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void)        __attribute__((weak));
void DebugMon_Handler(void)   __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void)  __attribute__((weak, alias("Default_Handler")));
void SysTick_Handler(void) __attribute__((weak, alias("Default_Handler")));

void Default_Handler(void) { while (1); }
void SVC_Handler(void)     { asm("bx lr"); }

#define IRQ_DEF Default_Handler

__attribute__((section(".isr_vector")))
void (*const g_pfnVectors[])(void) = {
    (void (*)(void))(0x20020000), /* 0  Initial MSP (top of QEMU 128K RAM) */
    Reset_Handler,                /* 1  Reset */
    NMI_Handler,                  /* 2  NMI */
    HardFault_Handler,            /* 3  HardFault */
    MemManage_Handler,            /* 4  MemManage */
    BusFault_Handler,             /* 5  BusFault */
    UsageFault_Handler,           /* 6  UsageFault */
    0, 0, 0, 0,                   /* 7-10 Reserved */
    SVC_Handler,                  /* 11 SVCall */
    DebugMon_Handler,             /* 12 Debug Monitor */
    0,                            /* 13 Reserved */
    PendSV_Handler,               /* 14 PendSV */
    SysTick_Handler,              /* 15 SysTick */

    /* STM32F407 peripheral IRQs 0..81 */
    IRQ_DEF, /* 0  WWDG */
    IRQ_DEF, /* 1  PVD */
    IRQ_DEF, /* 2  TAMP_STAMP */
    IRQ_DEF, /* 3  RTC_WKUP */
    IRQ_DEF, /* 4  FLASH */
    IRQ_DEF, /* 5  RCC */
    IRQ_DEF, /* 6  EXTI0 */
    IRQ_DEF, /* 7  EXTI1 */
    IRQ_DEF, /* 8  EXTI2 */
    IRQ_DEF, /* 9  EXTI3 */
    IRQ_DEF, /* 10 EXTI4 */
    IRQ_DEF, /* 11 DMA1_Stream0 */
    IRQ_DEF, /* 12 DMA1_Stream1 */
    IRQ_DEF, /* 13 DMA1_Stream2 */
    IRQ_DEF, /* 14 DMA1_Stream3 */
    IRQ_DEF, /* 15 DMA1_Stream4 */
    IRQ_DEF, /* 16 DMA1_Stream5 */
    IRQ_DEF, /* 17 DMA1_Stream6 */
    IRQ_DEF, /* 18 ADC */
    IRQ_DEF, /* 19 CAN1_TX */
    IRQ_DEF, /* 20 CAN1_RX0 */
    IRQ_DEF, /* 21 CAN1_RX1 */
    IRQ_DEF, /* 22 CAN1_SCE */
    IRQ_DEF, /* 23 EXTI9_5 */
    IRQ_DEF, /* 24 TIM1_BRK_TIM9 */
    IRQ_DEF, /* 25 TIM1_UP_TIM10 */
    IRQ_DEF, /* 26 TIM1_TRG_COM_TIM11 */
    IRQ_DEF, /* 27 TIM1_CC */
    IRQ_DEF, /* 28 TIM2 */
    IRQ_DEF, /* 29 TIM3 */
    IRQ_DEF, /* 30 TIM4 */
    IRQ_DEF, /* 31 I2C1_EV */
    IRQ_DEF, /* 32 I2C1_ER */
    IRQ_DEF, /* 33 I2C2_EV */
    IRQ_DEF, /* 34 I2C2_ER */
    IRQ_DEF, /* 35 SPI1 */
    IRQ_DEF, /* 36 SPI2 */
    IRQ_DEF, /* 37 USART1 */
    IRQ_DEF, /* 38 USART2 */
    IRQ_DEF, /* 39 USART3 */
    IRQ_DEF, /* 40 EXTI15_10 */
    IRQ_DEF, /* 41 RTC_Alarm */
    IRQ_DEF, /* 42 OTG_FS_WKUP */
    IRQ_DEF, /* 43 TIM8_BRK_TIM12 */
    IRQ_DEF, /* 44 TIM8_UP_TIM13 */
    IRQ_DEF, /* 45 TIM8_TRG_COM_TIM14 */
    IRQ_DEF, /* 46 TIM8_CC */
    IRQ_DEF, /* 47 DMA1_Stream7 */
    IRQ_DEF, /* 48 FSMC */
    IRQ_DEF, /* 49 SDIO */
    IRQ_DEF, /* 50 TIM5 */
    IRQ_DEF, /* 51 SPI3 */
    IRQ_DEF, /* 52 UART4 */
    IRQ_DEF, /* 53 UART5 */
    IRQ_DEF, /* 54 TIM6_DAC */
    IRQ_DEF, /* 55 TIM7 */
    IRQ_DEF, /* 56 DMA2_Stream0 */
    IRQ_DEF, /* 57 DMA2_Stream1 */
    IRQ_DEF, /* 58 DMA2_Stream2 */
    IRQ_DEF, /* 59 DMA2_Stream3 */
    IRQ_DEF, /* 60 DMA2_Stream4 */
    IRQ_DEF, /* 61 ETH */
    IRQ_DEF, /* 62 ETH_WKUP */
    IRQ_DEF, /* 63 CAN2_TX */
    IRQ_DEF, /* 64 CAN2_RX0 */
    IRQ_DEF, /* 65 CAN2_RX1 */
    IRQ_DEF, /* 66 CAN2_SCE */
    IRQ_DEF, /* 67 OTG_FS */
    IRQ_DEF, /* 68 DMA2_Stream5 */
    IRQ_DEF, /* 69 DMA2_Stream6 */
    IRQ_DEF, /* 70 DMA2_Stream7 */
    IRQ_DEF, /* 71 USART6 */
    IRQ_DEF, /* 72 I2C3_EV */
    IRQ_DEF, /* 73 I2C3_ER */
    IRQ_DEF, /* 74 OTG_HS_EP1_OUT */
    IRQ_DEF, /* 75 OTG_HS_EP1_IN */
    IRQ_DEF, /* 76 OTG_HS_WKUP */
    IRQ_DEF, /* 77 OTG_HS */
    IRQ_DEF, /* 78 DCMI */
    IRQ_DEF, /* 79 CRYP */
    IRQ_DEF, /* 80 HASH_RNG */
    IRQ_DEF, /* 81 FPU */
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
