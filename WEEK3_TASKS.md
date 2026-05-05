# Week 3 Task List

Status of [STM32F4_QEMU_FREERTOS_GUIDE.md](STM32F4_QEMU_FREERTOS_GUIDE.md) Week 3 with detail on remaining work.

---

## Completed

- [x] Week 1: Bare Metal Foundation (UART, SysTick, vector table at 0x08000000)
- [x] Week 2: FreeRTOS Integration & UART Command Interface (kernel ported, ring buffer, command parser, task manager, scheduler running)

---

## Remaining

### 1. Task State Machine Module

**Files (new):** `src/task_state_machine.c`, `src/task_state_machine.h`

**Why:** FreeRTOS tracks internal task state, but it doesn't expose transition history or "blocked on what" reasoning. A separate state machine lets us log lifecycle events and answer interview-grade questions about what each task is doing.

**Functions/structs to write:**

```c
typedef enum {
    TASK_STATE_INIT,
    TASK_STATE_READY,
    TASK_STATE_RUNNING,
    TASK_STATE_BLOCKED,
    TASK_STATE_SUSPENDED,
    TASK_STATE_DELETED
} TaskState;

typedef struct {
    char name[32];
    TaskState state;
    uint32_t priority;
    uint32_t creation_time;   // tick count at creation
    char blocked_on[32];      // e.g. "semaphore", "queue:cmd_q", "delay"
    int in_use;               // slot occupied?
} TaskStateInfo;

#define MAX_TRACKED_TASKS 16

void task_state_init(void);
int  task_state_add(const char *name, uint32_t priority);
int  task_state_transition(const char *name, TaskState new_state, const char *blocked_on);
const char *task_state_to_string(TaskState s);
void task_state_dump(void);                       // print all via uart_puts
TaskStateInfo *task_state_find(const char *name); // NULL if missing
```

**Validation rules in `task_state_transition()`:**

- Reject transitions on a task whose state is `TASK_STATE_DELETED`.
- Reject transition for a name that was never added.
- Log every accepted transition: `[STATE] <name>: <old> -> <new>`.

**Test (compile-only / host build):**

```bash
arm-none-eabi-gcc -c -I./src src/task_state_machine.c -o /tmp/tsm.o
# Must compile clean.

# Optional host smoke (compiles with native gcc since module has no FreeRTOS deps):
gcc -I./src -DTSM_HOST_TEST src/task_state_machine.c /tmp/tsm_smoke.c -o /tmp/tsm_smoke
/tmp/tsm_smoke
# Verify: add producer/consumer, run a few transitions, dump output matches.
```

---

### 2. Complete Command Handler

**File:** `src/command_parser.c`/`.h` and the `command_handler_task` in `src/main.c` (extend from Week 2)

**Why:** Week 2 stubbed only `TASK_CREATE` / `TASK_LIST` / `HELP`. Week 3 needs the full set so the user can drive the lifecycle from the UART.

**Commands to add to parser + handler:**

| Command | Action |
|---|---|
| `TASK_CREATE <name> <priority> [stack]` | `xTaskCreate()` + `task_state_add()` |
| `TASK_DELETE <name>` | `vTaskDelete()` + transition to `DELETED` |
| `TASK_LIST` | dump name, priority, state, stack high-water |
| `TASK_SUSPEND <name>` | `vTaskSuspend()` + transition to `SUSPENDED` |
| `TASK_RESUME <name>` | `vTaskResume()` + transition to `READY` |
| `TASK_INFO <name>` | print full `TaskStateInfo` for one task |
| `HELP` | print all commands |

**Guards:**

- Reject `TASK_SUSPEND IDLE` (suspending the idle task hangs the scheduler).
- Reject `TASK_CREATE` if name already exists or priority >= `configMAX_PRIORITIES`.
- After every command, print prompt `"> "`.

**Test:**

```bash
make clean && make all && make run

# In QEMU:
> HELP                       # full command list
> TASK_CREATE worker 2       # creates, prints [STATE] worker: INIT -> READY
> TASK_SUSPEND worker        # prints [STATE] worker: READY -> SUSPENDED
> TASK_RESUME worker         # prints [STATE] worker: SUSPENDED -> READY
> TASK_INFO worker           # full info dump
> TASK_SUSPEND IDLE          # must reject with error message
> TASK_DELETE worker         # prints [STATE] worker: ... -> DELETED
```

---

### 3. Example Tasks with Synchronization

**File:** `src/demo_tasks.c`, `src/demo_tasks.h` (new)

**Why:** A producer/consumer pair backed by a binary semaphore is the cleanest way to *see* FreeRTOS scheduling: the consumer blocks (no CPU spent), the producer unblocks it on a timer, priorities determine who runs first.

**Functions:**

```c
extern SemaphoreHandle_t g_demo_sem;   // created in main()

void demo_task_producer(void *pv);     // every 500ms: xSemaphoreGive(g_demo_sem)
void demo_task_consumer(void *pv);     // xSemaphoreTake(... portMAX_DELAY); ++count; print
void demo_task_low_priority(void *pv); // print "low_pri tick" every 1s; should starve when others busy
```

**Wiring in main.c:**

```c
g_demo_sem = xSemaphoreCreateBinary();
xTaskCreate(demo_task_producer,     "producer", 256, NULL, 3, NULL);
xTaskCreate(demo_task_consumer,     "consumer", 256, NULL, 4, NULL);
xTaskCreate(demo_task_low_priority, "low_pri",  256, NULL, 1, NULL);
```

**Test:**

```bash
make run

# Expected behaviour:
# - "consumer" prints once every ~500ms (woken by semaphore)
# - "low_pri" prints only when no higher task is ready
# - TASK_LIST shows consumer in BLOCKED state most of the time
```

---

### 4. Demonstrate Blocking vs. Polling

**File:** `src/demo_tasks.c` (extend)

**Why:** Show that `vTaskDelay()` yields the CPU (other tasks run) while a busy `while` loop with the same wall-clock duration starves them. This is the single most important Week 3 lesson.

**What to add:**

- A `demo_task_blocking()` that does `vTaskDelay(pdMS_TO_TICKS(100));` per iteration.
- A `demo_task_polling()` that does an empty `for (volatile int i = 0; i < N; ++i);` calibrated to ~100 ms.
- Print one line per iteration from each. Run both at the same priority.

**Test:**

```bash
make run

# With blocking version: low_pri still prints regularly.
# With polling version (toggle via #define): low_pri output starves while polling task is in its busy loop.
# Document this observation in a short comment block in demo_tasks.c.
```

---

### 5. Update Task Manager for State Machine

**File:** `src/task_manager.c` (modify from Week 2)

**Why:** The state machine only works if every create/delete/suspend/resume goes through one place. Hook it into `task_manager` so commands and demo tasks both update the tracker.

**Changes:**

- `task_manager_create()` → on success, call `task_state_add()` then `task_state_transition(name, READY, NULL)`.
- `task_manager_delete()` → call `task_state_transition(name, DELETED, NULL)` *before* `vTaskDelete()`.
- `task_manager_suspend()` / `_resume()` → wrap `vTaskSuspend` / `vTaskResume` and emit transitions.
- `task_manager_list()` → query `task_state_dump()` instead of building output ad-hoc.

**Who calls `task_state_transition()` for BLOCKED?**
The transitioning task itself, right before it calls a blocking primitive (e.g. inside `demo_task_consumer` before `xSemaphoreTake`). Document this in a header comment — it's a question the rubric asks.

**Test:**

```bash
make run

> TASK_CREATE worker 2
[STATE] worker: INIT -> READY
> TASK_SUSPEND worker
[STATE] worker: READY -> SUSPENDED
> TASK_LIST
# Output must come from task_state_dump() and show SUSPENDED.
> TASK_DELETE worker
[STATE] worker: SUSPENDED -> DELETED
> TASK_RESUME worker
# Must reject — task is DELETED.
```

---

### 6. Week 3 Expected Output

After all above, `make run` plus the following input must produce:

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

**Smoke check:**

```bash
# Drive QEMU stdin from a script, capture output for 5 seconds.
( printf "TASK_CREATE producer 3\nTASK_CREATE consumer 4\nTASK_LIST\n"; sleep 5 ) \
  | timeout 6 make run | tee /tmp/w3.log

grep -q "INIT -> READY"          /tmp/w3.log && \
grep -q "blocked on semaphore"   /tmp/w3.log && \
grep -q "state=RUNNING"          /tmp/w3.log
```

All three conditions pass → Week 3 done.

---

## Order of Attack

1. **Task state machine module** (pure logic; testable on host before integration).
2. **Hook state machine into task_manager** (single integration point for transitions).
3. **Complete command handler** (TASK_DELETE / SUSPEND / RESUME / INFO + guards).
4. **Demo tasks with semaphore** (producer/consumer/low_pri; observe priority preemption).
5. **Blocking vs. polling demo** (the conceptual payoff of the week).
6. **Smoke check** of the full Week 3 expected output.
