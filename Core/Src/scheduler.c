
#include "scheduler.h"

#define SCH_MAX_TASKS 16
#define SCH_INVALID_ID UINT32_MAX

typedef struct {
    void (*pFunction)(void);
    uint32_t Delay;
    uint32_t Period;
    uint32_t TaskID;
} SCH_Task;

static SCH_Task taskQueue[SCH_MAX_TASKS];
static uint32_t taskCount = 0;
static uint32_t nextTaskID = 1;
static volatile uint32_t systemTicks = 0;

static void Insert_Task(SCH_Task task) {
    uint32_t position = 0;
    uint32_t delay = task.Delay;
    while (position < taskCount && delay >= taskQueue[position].Delay) {
        delay -= taskQueue[position].Delay;
        position++;
    }
    for (uint32_t i = taskCount; i > position; i--) {
        taskQueue[i] = taskQueue[i - 1];
    }
    task.Delay = delay;
    taskQueue[position] = task;
    if (position + 1 < taskCount + 1) {
        taskQueue[position + 1].Delay -= delay;
    }
    taskCount++;
}

static void Remove_Task(uint32_t position) {
    if (position + 1 < taskCount) {
        taskQueue[position + 1].Delay += taskQueue[position].Delay;
    }
    for (uint32_t i = position; i + 1 < taskCount; i++) {
        taskQueue[i] = taskQueue[i + 1];
    }
    taskCount--;
}

void SCH_Init(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    taskCount = 0;
    nextTaskID = 1;
    systemTicks = 0;
    if (primask == 0) {
        __enable_irq();
    }
}

void SCH_Update(void) {
    systemTicks++;
    if (taskCount > 0 && taskQueue[0].Delay > 0) {
        taskQueue[0].Delay--;
    }
}

void SCH_Dispatch_Tasks(void) {
    while (1) {
        uint32_t primask = __get_PRIMASK();
        __disable_irq();
        if (taskCount == 0 || taskQueue[0].Delay > 0) {
            if (primask == 0) {
                __enable_irq();
            }
            break;
        }
        SCH_Task currentTask = taskQueue[0];
        Remove_Task(0);
        if (currentTask.Period > 0) {
            currentTask.Delay = currentTask.Period;
            Insert_Task(currentTask);
        }
        if (primask == 0) {
            __enable_irq();
        }
        currentTask.pFunction();
    }
}

uint32_t SCH_Add_Task(void (*pFunction)(void), uint32_t DELAY, uint32_t PERIOD) {
    if (pFunction == 0) {
        return SCH_INVALID_ID;
    }
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    if (taskCount >= SCH_MAX_TASKS) {
        if (primask == 0) {
            __enable_irq();
        }
        return SCH_INVALID_ID;
    }
    SCH_Task newTask;
    newTask.pFunction = pFunction;
    newTask.Delay = DELAY;
    newTask.Period = PERIOD;
    newTask.TaskID = nextTaskID++;
    if (nextTaskID == SCH_INVALID_ID) {
        nextTaskID = 1;
    }
    Insert_Task(newTask);
    uint32_t taskID = newTask.TaskID;
    if (primask == 0) {
        __enable_irq();
    }
    return taskID;
}

uint8_t SCH_Delete_Task(uint32_t taskID) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    for (uint32_t i = 0; i < taskCount; i++) {
        if (taskQueue[i].TaskID == taskID) {
            Remove_Task(i);
            if (primask == 0) {
                __enable_irq();
            }
            return 1;
        }
    }
    if (primask == 0) {
        __enable_irq();
    }
    return 0;
}

uint32_t get_time(void) {
    uint32_t primask = __get_PRIMASK();
    __disable_irq();
    uint32_t time = systemTicks * 10;
    if (primask == 0) {
        __enable_irq();
    }
    return time;
}
