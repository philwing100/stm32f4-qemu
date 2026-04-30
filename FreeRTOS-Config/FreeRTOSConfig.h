#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/* Map FreeRTOS Cortex-M port handlers onto vector table names */
#ifndef BARE_METAL
#define vPortSVCHandler        SVC_Handler
#define xPortPendSVHandler     PendSV_Handler
#define xPortSysTickHandler    SysTick_Handler
#endif

/* Core kernel settings */
#define configUSE_PREEMPTION                  1
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0
#define configUSE_TICKLESS_IDLE               0
#define configCPU_CLOCK_HZ                    168000000UL
#define configTICK_RATE_HZ                    1000UL
#define configMAX_PRIORITIES                  5
#define configMINIMAL_STACK_SIZE              128
#define configUSE_16_BIT_TICKS                0
#define configTOTAL_HEAP_SIZE                 8192

/* Features */
#define configUSE_MUTEXES                     1
#define configUSE_RECURSIVE_MUTEXES           1
#define configUSE_COUNTING_SEMAPHORES         1
#define configUSE_TASK_NOTIFICATIONS          1
#define configUSE_TIMERS                      1
#define configTIMER_TASK_PRIORITY             (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH              10
#define configTIMER_TASK_STACK_DEPTH          (configMINIMAL_STACK_SIZE * 2)

/* Debugging */
#define configUSE_IDLE_HOOK                   0
#define configUSE_TICK_HOOK                   0
#define configCHECK_FOR_STACK_OVERFLOW        2
#define configUSE_MALLOC_FAILED_HOOK          1

/* Interrupt priorities (ARM Cortex-M) */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY    15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5
#define configKERNEL_INTERRUPT_PRIORITY            (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << 4)
#define configMAX_SYSCALL_INTERRUPT_PRIORITY       (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << 4)

/* Memory allocation */
#define configSUPPORT_DYNAMIC_ALLOCATION      1
#define configSUPPORT_STATIC_ALLOCATION       0

/* Optional API includes */
#define INCLUDE_vTaskPrioritySet              1
#define INCLUDE_uxTaskPriorityGet             1
#define INCLUDE_vTaskDelete                   1
#define INCLUDE_vTaskSuspend                  1
#define INCLUDE_vTaskDelayUntil               1
#define INCLUDE_vTaskDelay                    1
#define INCLUDE_xTaskGetSchedulerState        1

#endif /* FREERTOS_CONFIG_H */
