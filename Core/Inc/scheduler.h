#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "main.h"
#include <stdint.h>

void SCH_Init(void);
void SCH_Update(void);
void SCH_Dispatch_Tasks(void);

uint32_t SCH_Add_Task(void (*pFunction)(void),
                      uint32_t DELAY,
                      uint32_t PERIOD);

uint8_t SCH_Delete_Task(uint32_t taskID);
uint32_t get_time(void);

#endif
