# Week 1 Task List

Status of [STM32F4_QEMU_FREERTOS_GUIDE.md](STM32F4_QEMU_FREERTOS_GUIDE.md) Week 1 with detail on remaining work.

---

## Completed

- [x] Install arm-none-eabi-gcc
- [x] Install QEMU with ARM support
- [x] Verify installation
- [x] Create project directory structure
- [x] Vector table listing CPU exceptions and interrupts
- [x] Reset handler initializes stack pointer
- [x] Copy .data section from FLASH to RAM
- [x] Zero .bss section
- [x] Weak exception handlers (NMI, HardFault, MemManage, BusFault, UsageFault)
- [x] Linker script defines FLASH and RAM regions
- [x] Linker places code/.rodata in FLASH
- [x] Linker places .data in RAM with FLASH load address
- [x] Linker places .bss in RAM
- [x] Stack at end of RAM (grows downward)
- [x] Makefile with `all`, `run`, `clean` targets
- [x] Builds and runs in QEMU

---

## Remaining

### 1. Move vector table to STM32 FLASH base 0x08000000

**File:** [stm32f407-qemu.ld](stm32f407-qemu.ld)

**Why:** Real STM32F407 boots from FLASH aliased to 0x00000000, but actual FLASH lives at 0x08000000. Guide requires the latter. QEMU `netduinoplus2` accepts both via aliasing.

**Change:**
```ld
FLASH (rx) : ORIGIN = 0x08000000, LENGTH = 1024K
```

**Test:**
```bash
arm-none-eabi-objdump -h freertos_hello.elf | grep .text
# Expect VMA/LMA = 08000000

arm-none-eabi-nm freertos_hello.elf | grep g_pfnVectors
# Expect 08000000

make run
# Must still boot and print
```

---

### 2. Register definitions header

**File (new):** `src/stm32f4_regs.h`

**Functions/structs to write:**

- `typedef struct { volatile uint32_t CR, PLLCFGR, CFGR, CIR, AHB1RSTR, ...; } RCC_TypeDef;`
- `typedef struct { volatile uint32_t MODER, OTYPER, OSPEEDR, PUPDR, IDR, ODR, BSRR, LCKR, AFR[2]; } GPIO_TypeDef;`
- `typedef struct { volatile uint32_t SR, DR, BRR, CR1, CR2, CR3, GTPR; } USART_TypeDef;`
- `typedef struct { volatile uint32_t ISER[8]; uint32_t RES0[24]; volatile uint32_t ICER[8]; ...; } NVIC_TypeDef;`
- `typedef struct { volatile uint32_t CTRL, LOAD, VAL, CALIB; } SysTick_TypeDef;`

**Base-address macros:**
```c
#define RCC      ((RCC_TypeDef  *) 0x40023800)
#define GPIOA    ((GPIO_TypeDef *) 0x40020000)
#define USART2   ((USART_TypeDef*) 0x40004400)
#define NVIC     ((NVIC_TypeDef *) 0xE000E100)
#define SYSTICK  ((SysTick_TypeDef*) 0xE000E010)
```

**Bit-mask macros:**
```c
#define RCC_AHB1ENR_GPIOAEN   (1U << 0)
#define RCC_APB1ENR_USART2EN  (1U << 17)
#define USART_SR_TXE          (1U << 7)
#define USART_CR1_UE          (1U << 13)
#define USART_CR1_TE          (1U << 3)
#define USART_CR1_RE          (1U << 2)
#define SYSTICK_CTRL_ENABLE   (1U << 0)
#define SYSTICK_CTRL_TICKINT  (1U << 1)
#define SYSTICK_CTRL_CLKSOURCE (1U << 2)
```

**Test (compile-only):**
```bash
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
```

---

### 3. UART driver (polling, real USART)

**File (new):** `src/uart.c` and `src/uart.h`

**Note:** QEMU `netduinoplus2` exposes USART2 on stdio (not USART1). Use USART2 / GPIOA pins PA2 (TX), PA3 (RX).

**Functions:**

```c
void uart_init(uint32_t baud);   // Enable RCC clocks, configure GPIOA AF7 on PA2/PA3,
                                 // set BRR for given baud (assume 16 MHz APB1), set CR1
                                 // bits UE|TE|RE.
void uart_putchar(char c);       // Wait until SR & TXE, then write DR.
void uart_puts(const char *s);   // Loop uart_putchar over string.
int  uart_getchar_blocking(void);// Wait until SR & RXNE, return DR.
```

**Baud divisor:** `USART2->BRR = pclk / baud;` (oversample 16). For QEMU pclk is treated as `configCPU_CLOCK_HZ / 4` per the FreeRTOSConfig (168 MHz / 4 = 42 MHz). Real number doesn't matter to QEMU — only register-touch sequence does.

**Update [src/main.c](src/main.c):** drop semihosting shortcut, call `uart_init(115200)` then `uart_puts(...)`.

**Test:**
```bash
make run
# Expect plain text on stdio (QEMU routes USART2 there).
# Required strings:
#   "=== STM32F4 Bare Metal Boot ==="
#   "UART initialized at 115200 baud"

# Negative test: comment out RCC clock-enable for USART2, rebuild.
# uart_putchar should hang (TXE never sets) → make run hangs → confirms
# polling logic depends on real peripheral state, not a bypass.
```

---

### 4. SysTick timer (1 ms tick, delay_ms)

**File (new):** `src/systick.c` and `src/systick.h`

**Functions:**

```c
void systick_init(uint32_t cpu_hz);   // SYSTICK->LOAD = cpu_hz/1000 - 1;
                                      // VAL = 0;
                                      // CTRL = CLKSOURCE | TICKINT | ENABLE;
void SysTick_Handler(void);           // ++g_ticks; (defined here, weak in startup)
uint32_t systick_now(void);           // return g_ticks
void delay_ms(uint32_t ms);           // poll until g_ticks - start >= ms
```

`g_ticks` = `static volatile uint32_t`.

**Conflict warning:** FreeRTOSConfig.h currently maps `SysTick_Handler` to `xPortSysTickHandler`. While doing the bare-metal Week 1 milestone, either:
- (a) temporarily comment out that #define, or
- (b) add `#ifndef BARE_METAL` around the FreeRTOS port-handler aliases and pass `-DBARE_METAL` for Week 1 builds.

**Test:**
```bash
# In main(): print "Tick..." every delay_ms(1000); ten times.
make run
# Expect ~10 "Tick..." lines spaced ~1 s of QEMU wall time.

# Verify reload value:
arm-none-eabi-objdump -d freertos_hello.elf | grep -A2 systick_init
# Expect immediate of (cpu_hz/1000 - 1) loaded into LOAD register.

# Verify counter increments:
# Add a uart_puts of g_ticks decimal each second. Should increase by ~1000.
```

---

### 5. Replace semihosting shortcut in main

**File:** [src/main.c](src/main.c)

After (3) and (4) work, remove `semihost_call()` and the `bkpt 0xAB` block. `uart_putchar` / `uart_puts` come from `uart.c`. Also drop `-semihosting` from Makefile run target.

**Test:**
```bash
grep -n bkpt src/main.c   # must return nothing
grep -n semihosting Makefile  # must return nothing
make clean && make all && make run
# Output must match expected Week 1 banner.
```

---

### 6. Week 1 expected output

After all above, `make run` must produce:

```
=== STM32F4 Bare Metal Boot ===
UART initialized at 115200 baud
SysTick timer running (1ms ticks)
Tick...
Tick...
Tick...
```

Smoke check:
```bash
timeout 5 make run | tee /tmp/w1.log
grep -q "Bare Metal Boot" /tmp/w1.log && \
grep -q "115200 baud"     /tmp/w1.log && \
grep -c "Tick..."         /tmp/w1.log    # >= 3
```

All three conditions pass → Week 1 done.

---

## Order of attack

1. Register header (no runtime risk; compile only).
2. Move vector table to 0x08000000 (linker change; small).
3. UART driver (must work before SysTick output is observable).
4. SysTick + delay_ms (depends on UART for proof).
5. Strip semihosting from main + Makefile.
6. Run smoke check.
