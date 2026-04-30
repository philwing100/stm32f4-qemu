#include <stdint.h>
//Functions/structs to write:

typedef struct {
    volatile uint32_t CR;
    volatile uint32_t PLLCFGR;
    volatile uint32_t CFGR;
    volatile uint32_t CIR;
    volatile uint32_t AHB1RSTR;
    volatile uint32_t AHB2RSTR;
    volatile uint32_t AHB3RSTR;
    uint32_t RESERVED0;
    volatile uint32_t APB1RSTR;
    volatile uint32_t APB2RSTR;
    uint32_t RESERVED1[2];
    volatile uint32_t AHB1ENR;
    volatile uint32_t AHB2ENR;
    volatile uint32_t AHB3ENR;
    uint32_t RESERVED2;
    volatile uint32_t APB1ENR;
    volatile uint32_t APB2ENR;
    uint32_t RESERVED3[2];
    volatile uint32_t AHB1LPENR;
    volatile uint32_t AHB2LPENR;
    volatile uint32_t AHB3LPENR;
    uint32_t RESERVED4;
    volatile uint32_t APB1LPENR;
    volatile uint32_t APB2LPENR;
    uint32_t RESERVED5[2];
    volatile uint32_t BDCR;
    volatile uint32_t CSR;
    uint32_t RESERVED6[2];
    volatile uint32_t SSCGR;
    volatile uint32_t PLLI2SCFGR;
} RCC_TypeDef;
typedef struct { volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2]; } GPIO_TypeDef;
typedef struct { volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR; } USART_TypeDef;
typedef struct {
    volatile uint32_t ISER[8];
    uint32_t RES0[24];
    volatile uint32_t ICER[8];
    uint32_t RES1[24];
    volatile uint32_t ISPR[8];
    uint32_t RES2[24];
    volatile uint32_t ICPR[8];
    uint32_t RES3[24];
    volatile uint32_t IABR[8];
    uint32_t RES4[56];
    volatile uint8_t  IP[240];
    uint32_t RES5[644];
    volatile uint32_t STIR;
} NVIC_TypeDef;
typedef struct { volatile uint32_t CTRL, LOAD, VAL, CALIB; } SysTick_TypeDef;


//Base-address macros:

#define RCC      ((RCC_TypeDef  *) 0x40023800)
#define GPIOA    ((GPIO_TypeDef *) 0x40020000)
#define USART2   ((USART_TypeDef*) 0x40004400)
#define NVIC     ((NVIC_TypeDef *) 0xE000E100)
#define SYSTICK  ((SysTick_TypeDef*) 0xE000E010)
//Bit-mask macros:

#define RCC_AHB1ENR_GPIOAEN   (1U << 0)
#define RCC_APB1ENR_USART2EN  (1U << 17)
#define USART_SR_TXE          (1U << 7)
#define USART_SR_RXNE         (1U << 5)
#define USART_CR1_UE          (1U << 13)
#define USART_CR1_TE          (1U << 3)
#define USART_CR1_RE          (1U << 2)
#define SYSTICK_CTRL_ENABLE   (1U << 0)
#define SYSTICK_CTRL_TICKINT  (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)


/*Test (compile-only):

arm-none-eabi-gcc -c -I./src src/stm32f4_regs.h -o /tmp/regs.o
# Must compile without warnings

cat > /tmp/regtest.c <<'EOF'
#include "stm32f4_regs.h"
int main(void){
    return (int)((uintptr_t)&RCC->AHB1ENR
               + (uintptr_t)&GPIOA->MODER
               + (uintptr_t)&USART2->SR);
}
EOF
arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -nostdlib -I./src -c /tmp/regtest.c -o /tmp/regtest.o
arm-none-eabi-objdump -d /tmp/regtest.o | grep -E '0x40023830|0x40020000|0x40004400'
# Must see correct register addresses in disassembly
*/