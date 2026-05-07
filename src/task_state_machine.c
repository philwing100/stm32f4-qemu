#include "task_state_machine.h"

/*typedef enum {
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

#define MAX_TRACKED_TASKS 16*/

void task_state_init(void){

}
int  task_state_add(const char *name, uint32_t priority){
    return 0;
}
int  task_state_transition(const char *name, TaskState new_state, const char *blocked_on){

}
const char *task_state_to_string(TaskState s){

}
void task_state_dump(void){

}                       
TaskStateInfo *task_state_find(const char *name){

}