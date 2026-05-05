# Week 4 Task List

Status of [STM32F4_QEMU_FREERTOS_GUIDE.md](STM32F4_QEMU_FREERTOS_GUIDE.md) Week 4 with detail on remaining work.

---

## Completed

- [x] Week 1: Bare Metal Foundation (UART, SysTick, vector table at 0x08000000)
- [x] Week 2: FreeRTOS Integration & UART Command Interface (kernel ported, ring buffer, command parser, task manager, scheduler running)
- [x] Week 3: Task State Machine & Synchronization (state tracking, full command set, semaphore demo, blocking vs polling)

---

## Remaining

### 1. Setup Google Test

**Files:** `tests/` directory (new), `tests/Makefile` or top-level `Makefile` target

**Why:** Unit tests run on the *host* (native gcc/g++), not in QEMU. Google Test gives us assertions, fixtures, and a runner — the same framework LM uses.

**What to do:**

- Install on the host:
  ```bash
  sudo apt-get install libgtest-dev cmake
  # On some distros gtest must be built from source:
  cd /usr/src/gtest && sudo cmake . && sudo make && \
    sudo cp lib/*.a /usr/lib
  ```
- Verify with a hello-world test:
  ```cpp
  // tests/smoke_test.cc
  #include <gtest/gtest.h>
  TEST(Smoke, Sanity) { EXPECT_EQ(1 + 1, 2); }
  ```
- Compile:
  ```bash
  g++ tests/smoke_test.cc -lgtest -lgtest_main -lpthread -o /tmp/smoke && /tmp/smoke
  # Expect: [  PASSED  ] 1 test.
  ```

---

### 2. Unit Tests for Command Parser

**File (new):** `tests/test_command_parser.cc`

**Why:** The parser is pure logic with no hardware deps. It should be the easiest module to hit 90%+ coverage on, and it's the headline "I can test embedded code" deliverable.

**Test cases (one `TEST()` each):**

```cpp
TEST(Parser, TaskCreateValid)         // "TASK_CREATE worker 2" → CMD_TASK_CREATE, name=worker, pri=2
TEST(Parser, TaskCreateWithStack)     // "TASK_CREATE worker 2 512" → stack_size=512
TEST(Parser, TaskListValid)           // "TASK_LIST" → CMD_TASK_LIST
TEST(Parser, HelpValid)               // "HELP" → CMD_HELP
TEST(Parser, UnknownCommand)          // "WALK_DOG" → CMD_UNKNOWN, returns -1
TEST(Parser, EmptyInput)              // "" → -1
TEST(Parser, WhitespaceOnly)          // "   \t  " → -1
TEST(Parser, ExtraWhitespace)         // "  TASK_LIST   " → CMD_TASK_LIST
TEST(Parser, MissingPriority)         // "TASK_CREATE worker" → -1
TEST(Parser, NameTooLong)             // 64-char name → truncated or rejected (document which)
TEST(Parser, PriorityOutOfRange)      // "TASK_CREATE w 999" → rejected
TEST(Parser, CaseSensitivity)         // "task_list" → document behaviour
```

**Build target in `Makefile`:**

```makefile
test-parser:
	g++ -I./src -Wall -Wextra \
	    tests/test_command_parser.cc src/command_parser.c \
	    -lgtest -lgtest_main -lpthread \
	    -o build/test_parser
	./build/test_parser
```

**Test:**

```bash
make test-parser
# Expect all tests pass.

# Coverage check (optional but worth it for the resume):
g++ --coverage -I./src tests/test_command_parser.cc src/command_parser.c \
    -lgtest -lgtest_main -lpthread -o build/test_parser_cov
./build/test_parser_cov
gcov src/command_parser.c
# Expect: >= 90% lines hit.
```

---

### 3. Unit Tests for State Machine

**File (new):** `tests/test_task_state_machine.cc`

**Why:** State machine is the other testable module. Validates transition logic without needing FreeRTOS.

**Use a fixture** to reset state between tests:

```cpp
class StateMachineTest : public ::testing::Test {
protected:
    void SetUp() override { task_state_init(); }
};

TEST_F(StateMachineTest, AddStoresTask)
TEST_F(StateMachineTest, AddDuplicateRejected)
TEST_F(StateMachineTest, TransitionUpdatesState)        // INIT -> READY -> RUNNING
TEST_F(StateMachineTest, TransitionToString)            // each enum value maps to string
TEST_F(StateMachineTest, DeletedTaskCannotTransition)   // DELETED is terminal
TEST_F(StateMachineTest, MissingTaskTransitionReturnsError)
TEST_F(StateMachineTest, MultipleTasksTrackedIndependently)
TEST_F(StateMachineTest, BlockedOnFieldStored)          // verify "blocked_on" persisted
TEST_F(StateMachineTest, OverflowMaxTrackedTasks)       // 17th task rejected
```

**Build target:**

```makefile
test-state:
	g++ -I./src -Wall -Wextra \
	    tests/test_task_state_machine.cc src/task_state_machine.c \
	    -lgtest -lgtest_main -lpthread \
	    -o build/test_state
	./build/test_state
```

**Test:**

```bash
make test-state
# Expect all pass; aim for 90%+ line coverage.
```

---

### 4. Build Test Executable (`make test`)

**File:** top-level `Makefile`

**Why:** Single command runs every host unit test. CI-friendly.

**Target:**

```makefile
.PHONY: test test-parser test-state
test: test-parser test-state
	@echo "All host unit tests passed."
```

Compile flags should include `-Wall -Wextra -Werror` for tests so warnings don't slip in.

**Test:**

```bash
make clean && make test
# Expect: parser + state machine suites all pass.
# Non-zero exit if any test fails (so CI can detect).
```

---

### 5. Integration Test on QEMU

**File (new):** `tests/integration_test.sh`

**Why:** Unit tests prove modules in isolation; integration test proves the *system* boots, accepts commands, and produces expected output end-to-end.

**Script outline:**

```bash
#!/bin/bash
set -e
LOG=$(mktemp)

# Drive QEMU stdin via a coprocess; capture stdout to LOG.
( printf "HELP\n";                sleep 1
  printf "TASK_CREATE worker 2\n"; sleep 1
  printf "TASK_LIST\n";            sleep 1
  printf "TASK_SUSPEND worker\n";  sleep 1
  printf "TASK_DELETE worker\n";   sleep 1
) | timeout 10 make run > "$LOG" 2>&1 || true

# Verify expected strings:
grep -q "=== Commands ==="          "$LOG" || { echo "FAIL: HELP";          exit 1; }
grep -q "INIT -> READY"             "$LOG" || { echo "FAIL: state add";     exit 1; }
grep -q "worker"                    "$LOG" || { echo "FAIL: TASK_LIST";     exit 1; }
grep -q "READY -> SUSPENDED"        "$LOG" || { echo "FAIL: SUSPEND";       exit 1; }
grep -q "-> DELETED"                "$LOG" || { echo "FAIL: DELETE";        exit 1; }

echo "Integration test PASSED"
```

**Makefile target:**

```makefile
.PHONY: integration-test
integration-test:
	bash tests/integration_test.sh
```

**Test:**

```bash
make integration-test
# Expect: "Integration test PASSED" and exit 0.
```

---

### 6. Documentation (README.md)

**File (new):** `README.md` at repo root

**Why:** This is the resume-facing artifact. An interviewer reads README before the code.

**Required sections:**

1. **Overview** — one paragraph: what it is, why it exists.
2. **Architecture diagram** — ASCII layered view:
   ```
   ┌──────────────────────────────────┐
   │  UART command shell              │  ← user-facing
   ├──────────────────────────────────┤
   │  command_parser  task_manager    │  ← testable logic
   │  task_state_machine  demo_tasks  │
   ├──────────────────────────────────┤
   │  FreeRTOS kernel + port layer    │  ← scheduler
   ├──────────────────────────────────┤
   │  uart.c  systick.c               │  ← drivers
   ├──────────────────────────────────┤
   │  startup.c  stm32f407-qemu.ld    │  ← bare metal
   ├──────────────────────────────────┤
   │  ARM Cortex-M4 (QEMU)            │  ← CPU
   └──────────────────────────────────┘
   ```
3. **File layout** — `src/`, `FreeRTOS/`, `FreeRTOS-Config/`, `tests/`, `stm32f407-qemu.ld`.
4. **Build & run** — prereqs, `make all`, `make run`, `make test`, `make integration-test`.
5. **UART command reference** — table of all commands with arguments and example output.
6. **Key concepts** — short paragraphs on: bare metal boot, vector table, RTOS scheduling, PendSV context switch, semaphores, blocking vs. polling.
7. **Debugging tips** — `arm-none-eabi-gdb`, `objdump -d`, `nm`, QEMU `-s -S`.
8. **Resources** — links from the guide's Resources section.
9. **Resume talking points** — bullet list mirroring the "Interview Talking Points" section of the guide.

**Test:**

```bash
# Manual review: ensure every Makefile target referenced in README actually works.
grep -oE 'make [a-z-]+' README.md | sort -u | while read cmd; do
  echo "Checking: $cmd"
  $cmd >/dev/null 2>&1 || echo "  BROKEN: $cmd"
done
```

---

### 7. Code Review & Cleanup

**Files:** all of `src/`

**Why:** A reviewer's eye on the whole tree before declaring done.

**Checklist:**

- [ ] Compile with `-Wall -Wextra -Werror`; fix every warning (do *not* silence with `-Wno-*`).
- [ ] `arm-none-eabi-objdump -t freertos_hello.elf | grep ' [a-z] '` — flag any `static` symbols never referenced.
- [ ] Search for `TODO`, `FIXME`, `XXX` — resolve or convert to GitHub issue.
- [ ] Run `cppcheck src/` for off-by-one and uninitialised-var warnings.
- [ ] Inspect every ISR for: short body, no blocking calls, no `printf`.
- [ ] Inspect every shared variable for `volatile` qualifier.
- [ ] Re-read assembly in `startup.c` and FreeRTOS port — add a comment block at the top of any non-obvious section.

**Test:**

```bash
make clean
CFLAGS_EXTRA="-Wall -Wextra -Werror" make all
# Expect: zero warnings, zero errors.

cppcheck --enable=warning,style --error-exitcode=1 src/
# Expect exit 0.
```

---

### 8. Final Build & Verification

**Why:** Single end-of-project smoke run.

**Sequence:**

```bash
make clean
make all                  # firmware builds, no warnings
make test                 # all unit tests green
make integration-test     # QEMU integration green
make run                  # interactive verification:
                          #   HELP, TASK_CREATE producer 3, TASK_CREATE consumer 4,
                          #   TASK_LIST, TASK_SUSPEND, TASK_RESUME, TASK_DELETE
```

**Definition of done:**

- [ ] `make all` produces firmware with zero warnings.
- [ ] `make test` shows all unit tests passing.
- [ ] `make integration-test` exits 0.
- [ ] All UART commands behave as documented in README.
- [ ] `git status` clean (no stray build artefacts uncommitted; `.gitignore` covers `*.o`, `*.elf`, `*.bin`, `*.map`, `build/`).
- [ ] README renders cleanly on GitHub.

---

## Order of Attack

1. **Google Test setup** (smoke test before writing real tests).
2. **Command parser tests** (easiest module, validates the framework works).
3. **State machine tests** (with fixture; aim 90%+ coverage on both).
4. **`make test` target** (single entry point).
5. **Integration test script** (ties unit + system level together).
6. **README.md** (needed before code cleanup so structure guides what gets cleaned).
7. **Code review & warning fixes** (tighten the whole tree).
8. **Final verification run** (clean → all → test → integration-test → manual run).

---

## Expected Artifacts by End of Week 4

- ✅ `tests/test_command_parser.cc` — passing, ≥90% coverage.
- ✅ `tests/test_task_state_machine.cc` — passing, ≥90% coverage.
- ✅ `tests/integration_test.sh` — passing.
- ✅ `Makefile` targets: `clean`, `all`, `run`, `test`, `integration-test`.
- ✅ `README.md` with architecture diagram, build/run, command reference, key concepts, resume talking points.
- ✅ Zero compiler warnings with `-Wall -Wextra -Werror`.
- ✅ Comments on all non-obvious sections (startup, port layer, ISRs).
