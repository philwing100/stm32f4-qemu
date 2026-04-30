# Week 2 Task List

Status of [STM32F4_QEMU_FREERTOS_GUIDE.md](STM32F4_QEMU_FREERTOS_GUIDE.md) Week 2 with detail on remaining work.

---

## Completed

- [x] Week 1: Bare Metal Foundation (UART, SysTick, vector table at 0x08000000)

---

## Remaining

### 1. Download & Port FreeRTOS

**Files:** `FreeRTOS/` directory (from GitHub), `FreeRTOS-Config/FreeRTOSConfig.h`

**Why:** FreeRTOS kernel is the heart of the RTOS. Configuration file tailors it to STM32F4 / ARM Cortex-M4.

**What to do:**

- Clone [FreeRTOS-Kernel](https://github.com/FreeRTOS/FreeRTOS-Kernel) into `FreeRTOS/` subdirectory (or add as submodule)
- Create `FreeRTOS-Config/FreeRTOSConfig.h` with:
  ```c
  #define configCPU_CLOCK_HZ                168000000  // 168 MHz
  #define configTICK_RATE_HZ                1000        // 1 kHz = 1 ms ticks
  #define configMAX_PRIORITIES              5
  #define configMINIMAL_STACK_SIZE           128        // words (512 bytes)
  #define configTOTAL_HEAP_SIZE             (50 * 1024) // 50 KB heap
  #define configUSE_PREEMPTION              1
  #define configUSE_TIME_SLICING            1
  #define configUSE_TASK_NOTIFICATIONS      1
  #define configUSE_QUEUES                  1
  #define configUSE_SEMAPHORES              1
  #define configUSE_MUTEXES                 1
  #define configSUPPORT_DYNAMIC_ALLOCATION  1
  #define configUSE_IDLE_HOOK               0
  #define configSVC_INTERRUPT_PRIORITY      2
  #define configPENDSV_INTERRUPT_PRIORITY   3
  #define configTIMER_INTERRUPT_PRIORITY    0
  #define configMAX_SYSCALL_INTERRUPT_PRIORITY 5
  ```

**Test:**

```bash
# Verify FreeRTOS files exist
ls -la FreeRTOS/
# Must contain: tasks.c, queue.c, list.c, portable/, ...

# Verify config compiles
arm-none-eabi-gcc -c -I./FreeRTOS-Config -I./FreeRTOS/include \
  FreeRTOS-Config/FreeRTOSConfig.h -o /tmp/config.o
# Must succeed without warnings
```

---

### 2. Understand the Port Layer

**Files:** `FreeRTOS/portable/GCC/ARM_CM4F/` (study, no changes yet)

**Why:** The port layer handles ARM-specific context switching and interrupt setup. Essential to understand before integrating.

**What to study:**

- `port.c`: Look for these functions:
  - `xPortPendSVHandler()` — context switcher ISR (PendSV exception)
  - `xPortSVCHandler()` — SVC exception handler (used by some FreeRTOS API calls)
  - `vPortSetupTimerInterrupt()` — configures SysTick for FreeRTOS
- `portmacro.h`: Defines ARM-specific types and macros (e.g., `portBASE_TYPE`, `portYIELD()`)

**Write brief notes (in comments or a document) on:**

1. How does PendSV enable context switching without calling scheduler from main?
2. What's the difference between SVC and PendSV?
3. Why does the port layer replace our bare-metal `SysTick_Handler` with FreeRTOS's?

**No code changes yet** — just understand the architecture.

---

### 3. Build FreeRTOS & Fix Compiler Errors

**File:** `Makefile` (update to include FreeRTOS)

**Why:** Link FreeRTOS kernel into the binary so tasks can actually run.

**What to do:**

- Update `Makefile` to add FreeRTOS source files to `SRCS`:
  ```makefile
  FREERTOS_SRC = FreeRTOS/tasks.c FreeRTOS/queue.c FreeRTOS/list.c \
                 FreeRTOS/timers.c FreeRTOS/stream_buffer.c \
                 FreeRTOS/croutine.c \
                 FreeRTOS/portable/GCC/ARM_CM4F/port.c \
                 FreeRTOS/portable/MemMang/heap_1.c

  SRCS = src/startup.c src/main.c src/uart.c src/systick.c $(FREERTOS_SRC)
  ```
- Add include paths:
  ```makefile
  CFLAGS += -I./FreeRTOS/include -I./FreeRTOS-Config
  ```
- Run `make clean && make all`
- Fix any compilation errors (usually: missing `stdint.h`, typos in config, wrong paths)

**Test:**

```bash
make clean && make all
# Must compile without errors. Warnings OK (FreeRTOS codebase may have some).

# Verify object files exist
ls -la FreeRTOS/*.o FreeRTOS/portable/GCC/ARM_CM4F/*.o FreeRTOS/portable/MemMang/*.o
# Must have compiled .o files
```

---

### 4. UART Interrupt-Driven RX & Ring Buffer

**Files:** `src/uart.c`, `src/uart.h` (modify from Week 1)

**Why:** Week 1 had polling RX (blocking). Now we use interrupts to receive characters asynchronously, enabling command input while other tasks run.

**What to add:**

- Ring buffer for RX data:
  ```c
  #define UART_RX_BUFFER_SIZE 256
  static volatile uint8_t uart_rx_buffer[UART_RX_BUFFER_SIZE];
  static volatile uint16_t uart_rx_head = 0;
  static volatile uint16_t uart_rx_tail = 0;
  ```
- Modify `uart_init()` to enable RX interrupt:
  ```c
  USART2->CR1 |= USART_CR1_RXNEIE;  // Enable RX not empty interrupt
  // Set NVIC priority for USART2_IRQ (usually priority 5+)
  NVIC_SetPriority(USART2_IRQn, 5);
  NVIC_EnableIRQ(USART2_IRQn);
  ```
- New ISR: `USART2_IRQHandler()`:
  ```c
  void USART2_IRQHandler(void) {
      if (USART2->SR & USART_SR_RXNE) {
          uint8_t c = USART2->DR;
          uart_rx_buffer[uart_rx_head] = c;
          uart_rx_head = (uart_rx_head + 1) % UART_RX_BUFFER_SIZE;
      }
  }
  ```
- New function: `uart_getchar_nb()` (non-blocking):
  ```c
  int uart_getchar_nb(void) {
      if (uart_rx_head == uart_rx_tail) return -1;  // empty
      uint8_t c = uart_rx_buffer[uart_rx_tail];
      uart_rx_tail = (uart_rx_tail + 1) % UART_RX_BUFFER_SIZE;
      return (int)c;
  }
  ```

**Test:**

```bash
# Rebuild and run
make run

# In QEMU terminal, type characters and verify they appear
# (If your app echoes them, you'll see them back)

# Verify interrupt is actually happening:
# Add a debug counter in USART2_IRQHandler, print it periodically.
# Counter should increase when you type.
```

---

### 5. Command Parser Module

**Files (new):** `src/command_parser.c`, `src/command_parser.h`

**Why:** Separate parsing logic from UART I/O, making it testable without hardware.

**What to write:**

- Header defines:
  ```c
  typedef enum {
      CMD_UNKNOWN,
      CMD_TASK_CREATE,
      CMD_TASK_LIST,
      CMD_TASK_SUSPEND,
      CMD_TASK_RESUME,
      CMD_TASK_DELETE,
      CMD_HELP
  } CommandType;

  typedef struct {
      CommandType type;
      char name[32];
      uint32_t priority;
      uint32_t stack_size;
  } Command;
  ```
- Function: `int parse_command(const char *line, Command *cmd);`
  - Parses lines like: `"TASK_CREATE worker 2"`, `"HELP"`, `"TASK_LIST"`
  - Returns 0 on success, -1 on error
  - Sets `cmd->type` and parameters

**Test (compile-only for now):**

```bash
# Verify header compiles
arm-none-eabi-gcc -c -I./src src/command_parser.c -o /tmp/parser.o
# Must succeed

# Manual test: write main.c snippet and verify parse_command() logic
cat > /tmp/parser_test.c <<'EOF'
#include "command_parser.h"
int main(void){
    Command cmd;
    parse_command("TASK_CREATE worker 2", &cmd);
    return cmd.priority;
}
EOF
# Compile and check it links without error
```

---

### 6. Task Manager Module

**Files (new):** `src/task_manager.c`, `src/task_manager.h`

**Why:** Centralize task creation/deletion/listing logic. Tracks task handles and metadata.

**What to write:**

- Header defines:
  ```c
  #define MAX_TASKS 16

  typedef struct {
      char name[32];
      TaskHandle_t handle;
      uint32_t priority;
      // Add: state, creation_time, etc. as needed
  } TaskInfo;

  void task_manager_init(void);
  int task_manager_create(const char *name, uint32_t priority, 
                          uint32_t stack_size);
  int task_manager_delete(const char *name);
  void task_manager_list(void);
  TaskHandle_t task_manager_get_handle(const char *name);
  ```
- Functions:
  - `task_manager_create()`: Call `xTaskCreate()`, store in local array
  - `task_manager_delete()`: Find task by name, call `vTaskDelete()`
  - `task_manager_list()`: Print all tracked tasks (name, priority, handle)
  - `task_manager_get_handle()`: Return handle for a task name

**Test:**

```bash
# Verify it compiles
arm-none-eabi-gcc -c -I./src -I./FreeRTOS/include \
  src/task_manager.c -o /tmp/task_mgr.o
# Must succeed
```

---

### 7. UART RX Task & Command Handler Task

**Files:** `src/main.c` (new tasks added)

**Why:** Tasks run concurrently. UART RX task reads commands; handler task executes them.

**What to add to main.c:**

- Task: `uart_rx_task()`:
  ```c
  static char line_buffer[256];
  static uint16_t line_idx = 0;

  void uart_rx_task(void *pvParameters) {
      (void)pvParameters;
      while (1) {
          int c = uart_getchar_nb();
          if (c >= 0) {
              if (c == '\n' || c == '\r') {
                  line_buffer[line_idx] = '\0';
                  // Pass line to handler (via queue or global)
                  line_idx = 0;
              } else if (line_idx < 255) {
                  line_buffer[line_idx++] = c;
              }
          }
          vTaskDelay(pdMS_TO_TICKS(10));
      }
  }
  ```
- Task: `command_handler_task()`:
  ```c
  void command_handler_task(void *pvParameters) {
      (void)pvParameters;
      Command cmd;
      while (1) {
          if (parse_command(line_buffer, &cmd) == 0) {
              switch (cmd.type) {
                  case CMD_TASK_CREATE:
                      task_manager_create(cmd.name, cmd.priority, cmd.stack_size);
                      break;
                  case CMD_TASK_LIST:
                      task_manager_list();
                      break;
                  case CMD_HELP:
                      uart_puts("=== Commands ===\n");
                      uart_puts("TASK_CREATE <name> <priority>\n");
                      uart_puts("TASK_LIST\nHELP\n");
                      break;
                  default:
                      uart_puts("Unknown command\n");
              }
          }
          vTaskDelay(pdMS_TO_TICKS(100));
      }
  }
  ```

**Test:**

```bash
make clean && make all
make run
# Type: HELP
# Expect: list of commands
# Type: TASK_LIST
# Expect: list of current tasks
```

---

### 8. Main with FreeRTOS

**File:** `src/main.c` (integrate kernel startup)

**Why:** Replace bare-metal main loop with FreeRTOS scheduler.

**What to change:**

- Initialize hardware (same as Week 1):
  ```c
  uart_init(115200);
  systick_init(configCPU_CLOCK_HZ);
  task_manager_init();
  ```
- Create FreeRTOS tasks:
  ```c
  xTaskCreate(uart_rx_task, "UART_RX", 256, NULL, 2, NULL);
  xTaskCreate(command_handler_task, "CMD_HANDLER", 256, NULL, 3, NULL);
  ```
- Start scheduler:
  ```c
  vTaskStartScheduler();
  // Code never reaches here
  ```

**Test:**

```bash
make clean && make all && make run

# Expected output:
# === STM32F4 FreeRTOS QEMU ===
# > (prompt appears, waiting for input)
# Type: HELP
# Expect: command list
# Type: TASK_LIST
# Expect: current tasks (UART_RX, CMD_HANDLER, idle)
# Type: TASK_CREATE worker 2
# Expect: task created message
```

---

## Order of Attack

1. **Download & Port FreeRTOS** (no compilation errors)
2. **Understand Port Layer** (study notes only)
3. **Build FreeRTOS** (link into binary, fix errors)
4. **UART Interrupt RX** (test with manual typing)
5. **Command Parser** (testable module, compile-check)
6. **Task Manager** (testable module, compile-check)
7. **UART RX Task & Command Handler** (add to main, test integration)
8. **Main with FreeRTOS** (startup scheduler, run full integration test)

---

## Week 2 Expected Output

```
=== STM32F4 FreeRTOS QEMU ===
Starting scheduler...
> HELP
=== Commands ===
TASK_CREATE <n> <priority> [stack_size]
TASK_LIST
TASK_SUSPEND <n>
TASK_RESUME <n>
HELP
> TASK_CREATE worker1 2
Creating task 'worker1' with priority 2
> TASK_LIST
=== Task List ===
Total: 3 tasks
[0] UART_RX (pri=2, state=READY)
[1] CMD_HANDLER (pri=3, state=READY)
[2] worker1 (pri=2, state=READY)
[3] IdleTask (pri=0, state=READY)
>
```
