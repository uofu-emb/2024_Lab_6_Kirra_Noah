#include "FreeRTOS.h"
#include "task.h"
#include "busy_threads.h"

void busy_busy(void *args)
{
    for (int i = 0; ; i++);
}

void busy_yield(void *args)
{
    for (int i = 0; ; i++) {
        taskYIELD();
    }
}