CROSS_COMPILE = arm-none-eabi-
CC = $(CROSS_COMPILE)gcc
OBJCOPY = $(CROSS_COMPILE)objcopy

CFLAGS = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -mfloat-abi=hard
CFLAGS += -Wall -Wextra -g -O0 -nostdlib -fno-builtin
CFLAGS += -I./FreeRTOS-Config -I./RTOS_Kernel/include
CFLAGS += -I./RTOS_Kernel/portable/GCC/ARM_CM4F

LDFLAGS = -mcpu=cortex-m4 -mthumb -T./stm32f407-qemu.ld
LDFLAGS += -Wl,-Map=output.map -Wl,--gc-sections

SRCS = src/main.c src/startup.c src/syscalls.c
SRCS += RTOS_Kernel/tasks.c RTOS_Kernel/queue.c RTOS_Kernel/list.c
SRCS += RTOS_Kernel/timers.c RTOS_Kernel/stream_buffer.c RTOS_Kernel/croutine.c
SRCS += RTOS_Kernel/portable/GCC/ARM_CM4F/port.c
SRCS += RTOS_Kernel/portable/MemMang/heap_1.c

OBJS = $(SRCS:.c=.o)

all: freertos_hello.bin

freertos_hello.elf: $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^

freertos_hello.bin: freertos_hello.elf
	$(OBJCOPY) -O binary $< $@
	@echo "✓ Built: $@"

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

run: freertos_hello.bin
	qemu-system-arm -machine versatilepb -m 128M -kernel $< -nographic

clean:
	rm -f $(OBJS) freertos_hello.elf freertos_hello.bin output.map

.PHONY: all run clean
