/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

#define LOW_TASK_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define MEDIUM_TASK_PRIORITY   ( tskIDLE_PRIORITY + 2UL )
#define HIGH_TASK_PRIORITY     ( tskIDLE_PRIORITY + 3UL )
#define LOW_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define MEDIUM_TASK_STACK_SIZE configMINIMAL_STACK_SIZE
#define HIGH_TASK_STACK_SIZE configMINIMAL_STACK_SIZE

SemaphoreHandle_t semaphore;

void high_priority_task(__unused void *params) {
    printf("Hello from high priority thread.\n");
    vTaskDelay(200);
    while(1){
        printf("Trying to take semaphore in high priority thread.\n");
        bool semaphore_pass = xSemaphoreTake(semaphore, portMAX_DELAY);
        if(semaphore_pass){
            printf("We have the semaphore in high priority!\n");
            for(int i = 0; i < 100000000; i++) {
                if (i % 10000000 == 0);
            }
            xSemaphoreGive(semaphore);
        }
    }
}

void medium_priority_task(__unused void *params) {
    printf("Hello from medium priority thread.\n");
    vTaskDelay(100);
    while(1){
        for(int i = 0; i < 1000000000; i++) {
            if (i % 100000000 == 0) {
                printf("Medium priority thread: counted to %d.\n", i);
            }
        }
    }
}

void low_priority_task(__unused void *params) {
    printf("Hello from low priority thread.\n");
    while(1){
        printf("Trying to take semaphore in low priority thread.\n");
        bool semaphore_pass = xSemaphoreTake(semaphore, 500);
        if(semaphore_pass){
            printf("We have the semaphore in low priority!\n");
            for(int i = 0; i < 100000000; i++) {
                if (i % 10000000 == 0);
            }
            xSemaphoreGive(semaphore);
        }
    }
}

int main( void )
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(10000); // Wait to connect to Pico
    printf("Main: starting code:\n");

    TaskHandle_t hp_task, mp_task, lp_task;
    // semaphore = xSemaphoreCreateBinary();
    // Using a mutex instead of a binary solves priority inversion 
    //  because it implements priority inheritence.
    semaphore = xSemaphoreCreateMutex(); 
    xSemaphoreGive(semaphore);
    xTaskCreate(low_priority_task, "Low Priority Thread",
                LOW_TASK_STACK_SIZE, NULL, LOW_TASK_PRIORITY, &lp_task);
    xTaskCreate(medium_priority_task, "Medium Priority Thread",
                MEDIUM_TASK_STACK_SIZE, NULL, MEDIUM_TASK_PRIORITY, &mp_task);
    xTaskCreate(high_priority_task, "High Priority Thread",
                HIGH_TASK_STACK_SIZE, NULL, HIGH_TASK_PRIORITY, &hp_task);
    
    printf("About to start task scheduler.\n");
    vTaskStartScheduler();
    return 0;
}
