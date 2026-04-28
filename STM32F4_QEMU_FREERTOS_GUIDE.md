# STM32F4 FreeRTOS QEMU Bare Metal Project: Task-Based Implementation Guide

A 4-week roadmap to build a bare metal task scheduler with UART interface on an emulated STM32F407VG using FreeRTOS and QEMU. This guide focuses on **what to learn and build**, with guiding questions to deepen your understanding.

---

## Project Overview & Learning Objectives

### The Big Picture

You're building an **interactive bare metal operating system simulator** that demonstrates:
- How a real embedded system boots (startup code, memory layout, interrupts)
- How a real-time OS schedules tasks and switches between them
- How to interact with hardware via registers (not HALs)
- How to test embedded code effectively

### Tech Stack

- **Toolchain**: arm-none-eabi-gcc (cross-compiler for ARM)
- **Simulator**: QEMU (runs your code without real hardware)
- **RTOS**: FreeRTOS (open-source real-time OS kernel)
- **Testing**: Google Test (unit test framework)
- **Target**: STM32F407VG (ARM Cortex-M4, 1MB FLASH, 192KB RAM)

### Resume Value

This project shows:
- ✅ You can write bare metal code (startup, ISRs, register I/O)
- ✅ You understand how RTOSes work internally (not just API calls)
- ✅ You design testable embedded systems (separating business logic from hardware)
- ✅ You can debug complex concurrent systems
- ✅ You match LM's actual tech stack (FreeRTOS, STM32, QEMU, Google Test)

---

## Week 1: Bare Metal Foundation

### Goal
Boot an ARM Cortex-M4 processor from cold reset through a barebones embedded application. Understand the CPU startup sequence and memory layout.

### Learning Outcomes

By the end of this week, you will understand:
- How an ARM CPU initializes at reset (vector table, stack pointer, program counter)
- The difference between FLASH (persistent code) and RAM (volatile data)
- How memory is laid out (code, initialized data, uninitialized data, stack/heap)
- How to write assembly code that the CPU actually executes
- How to configure peripheral clocks and use memory-mapped I/O registers
- How interrupt handlers work at the CPU level

### Week 1 Task Checklist

**Toolchain Setup**
- [ ] Install arm-none-eabi-gcc (cross-compiler)
- [ ] Install QEMU with ARM support
- [ ] Verify installation by checking version strings
- [ ] Create project directory structure

**Key Questions to Answer:**
- Why do we need a "cross-compiler" instead of a regular gcc?
- What does "arm-none-eabi" mean?
- Why does QEMU matter?

---

**Startup Code (Assembly)**
- [ ] Write a vector table that lists all CPU exceptions and interrupts
- [ ] Write reset handler that initializes the stack pointer
- [ ] Write code to copy .data section from FLASH to RAM
- [ ] Write code to zero the .bss section (uninitialized memory)
- [ ] Write weak handlers for exceptions (NMI, HardFault, etc.)

**Key Questions to Answer:**
- What is a vector table? Why does it have to be at address 0x0?
- What happens if you don't initialize the stack pointer?
- What's the difference between .data and .bss?
- Why write weak exception handlers?

---

**Linker Script**
- [ ] Define FLASH and RAM memory regions with correct addresses and sizes
- [ ] Place vector table at 0x08000000 (STM32F4 FLASH start)
- [ ] Place code and read-only data in FLASH
- [ ] Place initialized data (.data) in RAM (with FLASH as source)
- [ ] Place uninitialized data (.bss) in RAM
- [ ] Define stack location at end of RAM (grows downward)

**Key Questions to Answer:**
- Why does the linker script need to know memory addresses?
- What does "AT > FLASH" mean in a linker script?
- How does the CPU know where the stack starts?

---

**Register Definitions Header File**
- [ ] Define struct types for RCC (clock control), GPIO, USART, NVIC, SysTick
- [ ] Use volatile pointers so the compiler knows these can change at any time
- [ ] Define bit masks for enable flags, interrupt enables, status bits

**Key Questions to Answer:**
- Why use volatile for memory-mapped I/O?
- Why use pointers to structs for hardware registers?
- How do you know the register addresses?

---

**UART Driver (Basic, Polling)**
- [ ] Enable clock to GPIOA and USART1 peripherals via RCC registers
- [ ] Configure GPIO pins A9 (TX) and A10 (RX) as alternate function (USART mode)
- [ ] Calculate baud rate divisor for 115200 baud
- [ ] Initialize USART1: set baud rate, enable TX and RX, enable USART itself
- [ ] Implement uart_putchar(): poll TXE flag, write data register
- [ ] Implement uart_puts(): call uart_putchar() for each character

**Key Questions to Answer:**
- Why enable the clock before using a peripheral?
- What's an "alternate function"?
- How is baud rate calculated?
- Why poll TXE (TX Empty) flag?

---

**SysTick Timer**
- [ ] Configure SysTick to interrupt every 1ms (system tick)
- [ ] Write SysTick_Handler() to increment a global tick counter
- [ ] Implement delay_ms() that polls the tick counter

**Key Questions to Answer:**
- Why 1ms as the tick interval?
- How is the reload value calculated?
- What's the difference between interrupt-driven vs. polled timers?

---

**Main Entry Point & Build System**
- [ ] Write main() that initializes UART and SysTick
- [ ] Print "Hello from bare metal" to UART
- [ ] Create Makefile with: compile, link, run-on-QEMU targets
- [ ] Build and run on QEMU

**Key Questions to Answer:**
- What should main() do first?
- How do you redirect printf() to UART?
- Why does the Makefile need a .ld file?

### Expected Output by End of Week 1

```
=== STM32F4 Bare Metal Boot ===
UART initialized at 115200 baud
SysTick timer running (1ms ticks)
Tick...
Tick...
Tick...
```

---

## Week 2: FreeRTOS Integration & UART Command Interface

### Goal
Integrate the FreeRTOS kernel, understand how the scheduler works, and build a simple command interface via UART.

### Learning Outcomes

By the end of this week, you will understand:
- What a task (lightweight thread) is and how FreeRTOS creates/deletes them
- How the scheduler chooses which task runs (priority-based, preemptive)
- What context switching is and why the port layer is needed
- How interrupts (SysTick, UART RX) trigger the scheduler
- How FreeRTOS queues enable inter-task communication
- The difference between a ring buffer and a FreeRTOS queue

### Week 2 Task Checklist

**Download & Port FreeRTOS**
- [ ] Clone FreeRTOS-Kernel from GitHub (latest stable release)
- [ ] Create FreeRTOSConfig.h with configuration options
  - [ ] Set CPU clock, tick rate (1000Hz), max priorities, heap size
  - [ ] Set interrupt priorities for SVC, PendSV, SysTick
  - [ ] Enable dynamic task allocation, queues, semaphores
- [ ] Understand what each config option does (don't just copy-paste)

**Key Questions to Answer:**
- Why does FreeRTOS need a config file?
- What's configMAX_SYSCALL_INTERRUPT_PRIORITY?
- What does configTICK_RATE_HZ control?

---

**Understand the Port Layer**
- [ ] Study the ARM Cortex-M4 port implementation
  - [ ] What does vPortSetupTimerInterrupt() do?
  - [ ] What's xPortPendSVHandler()?
  - [ ] What's xPortSVCHandler()?
- [ ] Write brief notes on these three functions (in your own words)

**Key Questions to Answer:**
- Why does FreeRTOS need a custom SysTick handler?
- What's PendSV?
- Why can't the scheduler just run in main()?
- How does the kernel save/restore task state?

---

**Build FreeRTOS & Fix Compiler Errors**
- [ ] Update Makefile to include FreeRTOS source files
- [ ] Fix compilation issues (usually: missing includes, incorrect paths)
- [ ] Link the kernel into your binary

**Key Questions to Answer:**
- What compiler flags does FreeRTOS need?
- Why might linking fail?
- How do you debug linker errors?

---

**UART Interrupt-Driven RX & Ring Buffer**
- [ ] Modify uart_init() to enable RX interrupts in USART1 and NVIC
- [ ] Implement UART1_IRQHandler(): read data from DR, add to ring buffer
- [ ] Create a ring buffer: uart_rx_buffer[], head, tail pointers
- [ ] Implement uart_getchar_nb() (non-blocking): read from ring buffer without waiting

**Key Questions to Answer:**
- Why interrupt-driven instead of polling?
- What's a ring buffer?
- Why use head/tail pointers?

---

**Command Parser Module**
- [ ] Create command_parser.c with parse_command() function
- [ ] Parse commands like: "TASK_CREATE worker 2", "TASK_LIST", "HELP"
- [ ] Separate name, priority, stack size fields
- [ ] Return a Command struct with type and parameters
- [ ] Keep this module **testable** (no hardware dependencies)

**Key Questions to Answer:**
- Why separate the parser from UART?
- What commands do you need at minimum?
- How would you test the parser?
- What should happen for invalid input?

---

**Task Manager Module**
- [ ] Create task_manager.c to track running tasks
- [ ] Implement task_manager_create(): call xTaskCreate(), store in list
- [ ] Implement task_manager_list(): print all tasks with state and priority
- [ ] Implement task_manager_suspend/resume(): call FreeRTOS APIs
- [ ] Store task name, handle, priority, state for later queries

**Key Questions to Answer:**
- Why a separate task manager?
- What's a TaskHandle_t?
- How does task_manager track state?

---

**UART RX Task & Command Handler Task**
- [ ] Create uart_rx_task(): reads characters from ring buffer, builds lines
- [ ] On newline: display prompt, pass to command handler
- [ ] Create command_handler_task(): polls ring buffer, parses commands
- [ ] For each command, call task_manager functions or print help

**Key Questions to Answer:**
- Why separate UART RX into its own task?
- How do tasks communicate?
- What should happen if a command is invalid?

---

**Main with FreeRTOS**
- [ ] Initialize hardware (same as Week 1)
- [ ] Create FreeRTOS tasks: uart_rx_task, command_handler_task
- [ ] Create example worker task (to demonstrate task creation)
- [ ] Call vTaskStartScheduler() to begin task switching
- [ ] Test typing commands and seeing responses

**Key Questions to Answer:**
- What happens when you call xTaskCreate()?
- What does vTaskStartScheduler() do?
- Why create example tasks?

### Expected Output by End of Week 2

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
Total: 2 tasks
[0] worker1 (pri=2, state=RUNNING)
[1] IdleTask (pri=0, state=READY)
>
```

---

## Week 3: Task State Machine & Synchronization

### Goal
Build a deeper understanding of task lifecycle by tracking state transitions, and demonstrate FreeRTOS synchronization primitives (semaphores, queues).

### Learning Outcomes

By the end of this week, you will understand:
- The complete task lifecycle: INIT → READY → RUNNING → BLOCKED → SUSPENDED → DELETED
- What causes transitions (e.g., scheduler picks highest-priority ready task)
- How semaphores and queues make tasks wait for events
- The difference between blocking (task yields CPU) and polling (task wastes CPU)
- Why priority inversion is a problem and how FreeRTOS handles it

### Week 3 Task Checklist

**Task State Machine Module**
- [ ] Create task_state_machine.c to track detailed task states
  - [ ] Maintain array of TaskStateInfo: name, state, priority, creation_time, blocked_on
  - [ ] Implement task_state_add(): add task to tracking
  - [ ] Implement task_state_transition(): update state, log changes
  - [ ] Implement task_state_to_string(): convert state enum to string
  - [ ] Implement task_state_dump(): print all tasks with states
- [ ] Validate transitions (e.g., deleted tasks can't transition)

**Key Questions to Answer:**
- Why track state separately from FreeRTOS's internal state?
- What's "blocked_on"?
- Why log transitions?

---

**Complete Command Handler**
- [ ] Update command_handler_task to parse and execute all commands
  - [ ] TASK_CREATE: call xTaskCreate(), add to state machine
  - [ ] TASK_DELETE: delete task, remove from tracking
  - [ ] TASK_LIST: print all tasks (state, priority, stack usage)
  - [ ] TASK_SUSPEND: pause a task
  - [ ] TASK_RESUME: unpause a task
  - [ ] TASK_INFO: print state machine info for a task
  - [ ] HELP: show all available commands
- [ ] Add prompt ("> ") and error handling

**Key Questions to Answer:**
- What should TASK_LIST show?
- What happens if you suspend the idle task?
- How do you prevent that?

---

**Example Tasks with Synchronization**
- [ ] Implement demo_task_producer(): signals a semaphore every 500ms
- [ ] Implement demo_task_consumer(): waits on semaphore, increments counter
- [ ] Implement demo_task_low_priority(): low-priority task that prints periodically
- [ ] Create semaphore in main(): xSemaphoreCreateBinary()
- [ ] Observe priority scheduling: high-priority tasks preempt low-priority

**Key Questions to Answer:**
- What's a semaphore?
- Why use xSemaphoreCreateBinary()?
- What happens when consumer waits but producer hasn't signaled?
- How does priority-based scheduling work?

---

**Demonstrate Blocking**
- [ ] Add vTaskDelay() calls to show blocking (task yields CPU)
- [ ] Contrast with polling (busy-waiting without yielding)
- [ ] Observe in UART output that blocked tasks don't consume CPU

**Key Questions to Answer:**
- What's vTaskDelay()?
- What's the difference between vTaskDelay() and a while loop?
- How does the scheduler know when a delayed task is ready?

---

**Update Task Manager for State Machine**
- [ ] Integrate state machine into task_manager_create() and task_manager_delete()
- [ ] Log every transition
- [ ] Update task_manager_list() to show state machine info

**Key Questions to Answer:**
- When should you call task_state_add()?
- When should you call task_state_transition()?
- Who calls task_state_transition()?

### Expected Output by End of Week 3

```
> TASK_CREATE producer 3
[STATE] producer: INIT -> READY
Creating task 'producer' with priority 3
> TASK_CREATE consumer 4
[STATE] consumer: INIT -> READY
Creating task 'consumer' with priority 4
> TASK_LIST
=== Task List ===
[0] producer (pri=3, state=RUNNING)
[1] consumer (pri=4, state=BLOCKED) (blocked on semaphore)
[2] low_pri (pri=1, state=READY)
>
```

---

## Week 4: Testing & Polish

### Goal
Write comprehensive tests for your testable modules, document the system, and ensure the project is clean and professional.

### Learning Outcomes

By the end of this week, you will understand:
- How to test embedded code without hardware (unit tests for logic)
- How to integrate tests with Google Test framework
- How to write integration tests that exercise the full system
- How to document an embedded project for resume/interview
- Best practices for code organization and clarity

### Week 4 Task Checklist

**Setup Google Test**
- [ ] Install Google Test development libraries
- [ ] Verify installation: can compile a simple test
- [ ] Understand Google Test syntax: TEST(), TEST_F(), EXPECT_EQ(), etc.

**Key Questions to Answer:**
- What's a unit test?
- Why Google Test over other frameworks?
- What's the difference between TEST and TEST_F?

---

**Unit Tests for Command Parser**
- [ ] Write tests for parse_command():
  - [ ] Valid TASK_CREATE command
  - [ ] Valid TASK_LIST command
  - [ ] Invalid/unknown command
  - [ ] Empty input
  - [ ] Extra whitespace
  - [ ] Missing parameters
  - [ ] Edge cases (very long name, very high priority)
- [ ] Aim for 90%+ line coverage

**Key Questions to Answer:**
- How do you test the parser without QEMU?
- What's a "fixture"?
- Why test edge cases?
- How many tests is enough?

---

**Unit Tests for State Machine**
- [ ] Write tests for task_state_machine.c:
  - [ ] task_state_add() stores task
  - [ ] task_state_transition() updates state correctly
  - [ ] task_state_to_string() returns right names
  - [ ] Invalid transitions are rejected
  - [ ] Querying non-existent task returns error
  - [ ] Multiple tasks tracked independently
- [ ] Aim for 90%+ line coverage

**Key Questions to Answer:**
- How do you set up a state machine for each test?
- What's the difference between testing "add" and "transition"?
- How do you test error cases?

---

**Build Test Executable**
- [ ] Create Makefile target: "make test"
- [ ] Compile command_parser.c and task_state_machine.c with test files
- [ ] Link with Google Test libraries
- [ ] Run tests and verify all pass

**Key Questions to Answer:**
- Why compile .c files separately for tests?
- What compiler flags for tests?
- How do you verify tests passed?

---

**Integration Test on QEMU**
- [ ] Write shell script that:
  - [ ] Boots QEMU with your app
  - [ ] Sends command sequences: HELP, TASK_CREATE, TASK_LIST, etc.
  - [ ] Verifies output contains expected strings
  - [ ] Times out after N seconds (to avoid hanging)
- [ ] Add target: "make integration-test"

**Key Questions to Answer:**
- How do you send commands to QEMU automatically?
- How do you verify output?
- Why timeout?
- What commands should you test?

---

**Documentation**
- [ ] Write comprehensive README.md:
  - [ ] Project overview and learning objectives
  - [ ] Architecture diagram (layers: command → scheduler → drivers → bare metal → CPU)
  - [ ] File organization (what's in src/, linker/, FreeRTOS/, tests/)
  - [ ] Build instructions (prerequisites, compilation, running)
  - [ ] Command reference (all UART commands)
  - [ ] Key concepts (bare metal, FreeRTOS, testing, ARM architecture)
  - [ ] Debugging tips (GDB, printf, QEMU)
  - [ ] Learning resources (links to datasheets, FreeRTOS docs, ARM docs)
  - [ ] Resume talking points (what this demonstrates)
- [ ] Add comments to complex code sections
- [ ] Ensure code follows consistent style

**Key Questions to Answer:**
- Who reads the README?
- What should be in architecture section?
- How do you explain bare metal concepts?
- What's a "resume talking point"?

---

**Code Review & Cleanup**
- [ ] Check for:
  - [ ] Unused variables or functions
  - [ ] Compiler warnings (compile with -Wall -Wextra)
  - [ ] Memory leaks
  - [ ] Off-by-one errors
  - [ ] Uninitialized variables
- [ ] Refactor for clarity (rename confusing variables, split large functions)
- [ ] Add helpful comments (especially in assembly and ISRs)

**Key Questions to Answer:**
- What's the most common embedded bug?
- How do you avoid these bugs?
- When should you add comments?

---

**Final Build & Verification**
- [ ] Build everything: `make clean`, `make all`
- [ ] Run on QEMU: `make run`
- [ ] Run unit tests: `make test`
- [ ] Run integration tests: `make integration-test`
- [ ] Verify no compiler warnings
- [ ] Test all UART commands manually

**Key Questions to Answer:**
- What's your definition of "done"?
- What would you do differently next time?
- What would you add with more time?

### Expected Artifacts by End of Week 4

- ✅ Passing unit tests for parser and state machine
- ✅ Passing integration test on QEMU
- ✅ README.md with architecture, instructions, concepts
- ✅ Makefile with clean, all, run, test, integration-test targets
- ✅ No compiler warnings
- ✅ Code formatted consistently
- ✅ Comments on complex sections (startup.s, port layer, ISRs)

---

## Interview Talking Points

By the end of this project, you can discuss:

### "Tell me about bare metal programming"
- Startup code copies data sections, zeros BSS, jumps to main
- Memory layout defined by linker script
- Registers are memory-mapped I/O; write address = call hardware
- Vector table at fixed address; CPU jumps there on reset/exception

### "How does an RTOS work?"
- Kernel maintains task control blocks (TCBs) with stack, registers, state
- Scheduler picks highest-priority ready task
- Context switch: ISR saves CPU state, restores different task's state
- SysTick interrupt (every 1ms) allows preemption

### "How did you test this?"
- Command parser testable without hardware (Google Test)
- State machine logic testable independently
- Integration tests run full system on QEMU with command sequences
- Separation of concerns: business logic vs. hardware drivers

### "What was the hardest part?"
- Toolchain setup (cross-compiler, linker script, QEMU configuration)
- Understanding the port layer (assembly context switching)
- Debugging with GDB when scheduler makes timing unpredictable
- Designing testable architecture (extracting logic from hardware)

### "Why this project for LM?"
- Demonstrates core embedded skills: bare metal, RTOS, interrupts, testing
- Matches LM's stack: FreeRTOS, STM32, Google Test
- Shows ability to learn from datasheets and official docs
- Proves understanding of how systems actually work, not just API usage

---

## Common Pitfalls & How to Avoid Them

### Pitfall 1: Linker Script Errors
**Problem**: "undefined reference to `_stack_end`"
**Root Cause**: Linker script symbols not defined  
**Solution**: Double-check linker script syntax; use `arm-none-eabi-nm` to inspect object files

### Pitfall 2: UART Not Printing
**Problem**: QEMU runs but no output  
**Root Cause**: Clock not enabled, GPIO not configured, baud rate wrong  
**Solution**: Enable RCC clock, set GPIO alternate function, verify baud rate math

### Pitfall 3: Hard Fault at Startup
**Problem**: QEMU starts, then crashes with HardFault  
**Root Cause**: Uninitialized memory, infinite loop in startup, missing vector table entry  
**Solution**: Check vector table alignment, verify startup code logic

### Pitfall 4: Scheduler Doesn't Start
**Problem**: FreeRTOS code compiles but scheduler never runs  
**Root Cause**: Port layer not linked, SysTick not configured, ISR priorities wrong  
**Solution**: Verify FreeRTOS source in Makefile, check SysTick interrupt enabled, use GDB to step through startup

### Pitfall 5: Tasks Never Run
**Problem**: Create task but it never executes  
**Root Cause**: Scheduler not running, task priority too low, task immediately blocks  
**Solution**: Add debug output to confirm scheduler running, check idle task runs, verify task function doesn't block immediately

---

## Resources & References

### Essential Documentation
- [ARM Cortex-M4 Technical Reference Manual](https://developer.arm.com/documentation/100166/latest/)
- [STM32F407 Datasheet](https://www.st.com/resource/en/datasheet/dm00037051.pdf)
- [STM32F407 Reference Manual](https://www.st.com/resource/en/reference_manual/dm00031020.pdf)
- [FreeRTOS Official Documentation](https://www.freertos.org/FreeRTOS-quick-start-guide.html)
- [QEMU ARM Board Support](https://qemu-project.gitlab.io/qemu/system/arm/stm32.html)
- [Google Test Tutorial](https://google.github.io/googletest/)

### Learning Resources
- YouTube: "Embedded Systems Programming" courses (search for bare metal ARM)
- FreeRTOS: Official YouTube channel has kernel/scheduler walkthroughs
- Book: "The Definitive Guide to ARM Cortex-M4" by Joseph Yiu

### Tools
- **arm-none-eabi-gdb**: Debugger; can step through code in QEMU
- **objdump**: Inspect binary; see disassembly of compiled code
- **objcopy**: Convert ELF to binary/hex for flashing
- **nm**: List symbols; check what's in object files
