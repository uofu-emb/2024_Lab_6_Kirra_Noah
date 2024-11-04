/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include <semphr.h>
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

int count = 0;
bool on = false;

#define LOW_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define HIGH_TASK_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define LOW_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define HIGH_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

SemaphoreHandle_t semaphore;

void high_priority_task(__unused void *params) {
    vTaskDelay(500);
    while(1){
        bool semaphore_pass = xSemaphoreTake(semaphore, 500);
        if(semaphore_pass){
            printf("Hello World! We are in high priority!");
            xSemaphoreGive(semaphore);
        }
    }
}

void low_priority_task(__unused void *params) {
    while(1){
        bool semaphore_pass = xSemaphoreTake(semaphore, 500);
        if(semaphore_pass){
            printf("Hello World! We are in low priority!");
            vTaskDelay(1000);
            xSemaphoreGive(semaphore);
        }
    }
}

int main( void )
{
    stdio_init_all();

    TaskHandle_t hp_task, lp_task;
    semaphore = xSemaphoreCreateBinary();
    xTaskCreate(low_priority_task, "LOWThread",
                LOW_TASK_STACK_SIZE, NULL, LOW_TASK_PRIORITY, &lp_task);
    xTaskCreate(high_priority_task, "HIGHThread",
                HIGH_TASK_STACK_SIZE, NULL, HIGH_TASK_PRIORITY, &hp_task);
    vTaskStartScheduler();
    return 0;
}
