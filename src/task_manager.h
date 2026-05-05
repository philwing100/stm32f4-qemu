#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include <stdint.h>
#include "FreeRTOS.h"
#include "task.h"
#include "uart.h"

#define MAX_TASKS 16

typedef struct {
    char name[32];
    uint32_t priority;
    uint32_t stack_size;
    TaskHandle_t handle;

    // Add: state, creation_time, etc. as needed
} TaskInfo;


void task_manager_init(void);
int task_manager_create(const char *name, uint32_t priority, uint32_t stack_size);
int task_manager_delete(const char *name);
void task_manager_list(void);
TaskHandle_t task_manager_get_handle(const char *name);

#endif