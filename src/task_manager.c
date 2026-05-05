#include "task_manager.h"
#include "stm32f4_regs.h"
#include <string.h>   // strcmp, strncpy
#include <stdio.h>  


static TaskInfo tasks[MAX_TASKS];
static uint8_t size= 0;

void task_manager_init(void){
   memset(tasks, 0, sizeof(tasks));
    size = 0;
}

static void generic_task(void *pvParameters) {
    (void)pvParameters;
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // idle worker
    }
}

TaskHandle_t task_manager_get_handle(const char *name){
    for(uint32_t i =0; i< size;i++){
        if(strcmp(tasks[i].name, name) == 0){
            return tasks[i].handle;
        }
    }
    return NULL; //null handle
}

int task_manager_create(const char *name, uint32_t priority, uint32_t stack_size) {
    if (size >= MAX_TASKS) return -1;
    TaskHandle_t handle = NULL;
    BaseType_t ok = xTaskCreate(generic_task, name, stack_size, NULL, priority, &handle);
    if (ok != pdPASS) return -1;
    strncpy(tasks[size].name, name, sizeof(tasks[size].name) - 1);
    tasks[size].handle  = handle;
    tasks[size].priority = priority;
    tasks[size].stack_size = stack_size;
    size++;
    return 0;
}

int task_manager_delete(const char *name){
    for (uint32_t i = 0; i < size; i++) {
    if (strcmp(tasks[i].name, name) == 0) {
        vTaskDelete(tasks[i].handle);
        for (uint32_t j = i; j < size - 1; j++) {
            tasks[j] = tasks[j + 1];
        }
        size--;
        return 0;
    }
}
return -1;
}

void task_manager_list(void){
    for(uint32_t i =0; i< size;i++){    
        uart_puts(tasks[i].name);
    }
}

/*

task_manager_create(): Call xTaskCreate(), store in local array
task_manager_delete(): Find task by name, call vTaskDelete()
task_manager_list(): Print all tracked tasks (name, priority, handle)
task_manager_get_handle(): Return handle for a task name
*/