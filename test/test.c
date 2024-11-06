#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <FreeRTOS.h>
#include <pico/multicore.h>
#include <pico/cyw43_arch.h>
#include "busy_threads.h"

#define TEST_RUNNER_PRIORITY    (tskIDLE_PRIORITY + 5UL)
#define TEST_THREAD_MEDIUM_PRIORITY  (TEST_RUNNER_PRIORITY - 1UL)
#define TEST_THREAD_LOW_PRIORITY  (TEST_RUNNER_PRIORITY - 2UL)
#define TEST_THREAD_STACK_SIZE  (configMINIMAL_STACK_SIZE)

void setUp(void) {}

void tearDown(void) {}

void test_same_priority_busy_busy(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_busy, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_a);
    xTaskCreate(busy_busy, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_b);

    vTaskDelay(1000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("Thread thread A has run for %lu ticks.\n", a_run_time);
    printf("Thread thread B has run for %lu ticks.\n", b_run_time);
}

void test_same_priority_yield_yield(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_yield, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_a);
    xTaskCreate(busy_yield, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_b);

    vTaskDelay(1000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("Thread thread A has run for %lu ticks.\n", a_run_time);
    printf("Thread thread B has run for %lu ticks.\n", b_run_time);
}

// void test_side_thread_function(void)
// {
//     SemaphoreHandle_t semaphore;
//     semaphore = xSemaphoreCreateCounting(1, 1);
//     int counter = 4;
//     TEST_ASSERT_TRUE_MESSAGE(side_thread_function(&counter, semaphore) == pdTRUE, "Function failed to acquire semaphore.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 5, "Thread failed to increment counter.");
// }

// void test_side_thread_semaphore(void)
// {
//     SemaphoreHandle_t semaphore;
//     semaphore = xSemaphoreCreateCounting(1, 1);
//     xSemaphoreTake(semaphore, 500);
//     int counter = 4;
//     TEST_ASSERT_TRUE_MESSAGE(side_thread_function(&counter, semaphore) == pdFALSE, "Function acquired semaphore when it should not have.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 4, "Thread changed counter when it should not have.");
// }

// void test_main_thread_semaphore(void)
// {
//     SemaphoreHandle_t semaphore;
//     semaphore = xSemaphoreCreateCounting(1, 1);
//     xSemaphoreTake(semaphore, 500);
//     int counter = 4;
//     int on = 1;
//     TEST_ASSERT_TRUE_MESSAGE(main_thread_function(&counter, &on, semaphore) == pdFALSE, "Function acquired semaphore when it should not have.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 4, "Thread changed counter when it should not have.");
//     TEST_ASSERT_TRUE_MESSAGE(on == 1, "Thread toggled on when it should not have.");
// }

// void test_deadlock(void)
// {
//     TaskHandle_t thread_a, thread_b;
//     SemaphoreHandle_t semaphore_a, semaphore_b;

//     semaphore_a = xSemaphoreCreateCounting(1, 1);
//     semaphore_b = xSemaphoreCreateCounting(1, 1);

//     struct DeadlockParams deadlock_params_a = {semaphore_a, semaphore_b, 4};
//     struct DeadlockParams deadlock_params_b = {semaphore_b, semaphore_a, 4};
    
//     printf("Creating threads a and b.\n");
//     xTaskCreate(thread_deadlock_function, "ThreadA",
//                 TEST_THREAD_STACK_SIZE, (void *)&deadlock_params_a, TEST_THREAD_A_PRIORITY, &thread_a);
//     xTaskCreate(thread_deadlock_function, "ThreadB",
//                 TEST_THREAD_STACK_SIZE, (void *)&deadlock_params_b, TEST_THREAD_B_PRIORITY, &thread_b);
    
//     printf("Waiting for threads a and b to deadlock. . .\n");
//     vTaskDelay(1000);
//     vTaskSuspend(thread_a);
//     vTaskSuspend(thread_b);

//     printf("Threads a and b suspended. Testing variables. . .\n");
//     TEST_ASSERT_TRUE_MESSAGE(uxSemaphoreGetCount(semaphore_a) == 0, "Semaphore A did not cause a deadlock.");
//     TEST_ASSERT_TRUE_MESSAGE(uxSemaphoreGetCount(semaphore_b) == 0, "Semaphore B did not cause a deadlock.");
//     TEST_ASSERT_FALSE_MESSAGE(deadlock_params_a.testvar <= 4, "Thread a did not acquire semaphore a.");
//     TEST_ASSERT_FALSE_MESSAGE(deadlock_params_a.testvar >= 6, "Deadlock did not occur; test variable in thread a was incremented twice.");
//     TEST_ASSERT_FALSE_MESSAGE(deadlock_params_b.testvar <= 4, "Thread b did not acquire semaphore b.");
//     TEST_ASSERT_FALSE_MESSAGE(deadlock_params_b.testvar >= 6, "Deadlock did not occur; test variable in thread b was incremented twice.");

//     printf("Killing threads a and b.\n");
//     vTaskDelete(thread_a);
//     vTaskDelete(thread_b);
// }

// void test_orphaned_lock(void)
// {
//     SemaphoreHandle_t semaphore;
//     semaphore = xSemaphoreCreateCounting(1, 1);
//     int counter = 4;
//     TEST_ASSERT_TRUE_MESSAGE(orphaned_lock(&counter, semaphore) == pdTRUE, "Function failed to acquire semaphore.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 5, "Thread failed to increment counter.");

//     TEST_ASSERT_TRUE_MESSAGE(orphaned_lock(&counter, semaphore) == pdFALSE, "Function acquired semaphore when it should not have.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 5, "Thread changed counter when it should not have.");

//     TEST_ASSERT_TRUE_MESSAGE(orphaned_lock(&counter, semaphore) == pdFALSE, "Function acquired semaphore when it should not have.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 5, "Thread changed counter when it should not have.");
// }

// void test_not_orphaned_lock(void)
// {
//     SemaphoreHandle_t semaphore;
//     semaphore = xSemaphoreCreateCounting(1, 1);
//     int counter = 4;
//     TEST_ASSERT_TRUE_MESSAGE(not_orphaned_lock(&counter, semaphore) == pdTRUE, "Function failed to acquire semaphore.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 5, "Thread failed to increment counter.");

//     TEST_ASSERT_TRUE_MESSAGE(not_orphaned_lock(&counter, semaphore) == pdTRUE, "Function acquired semaphore when it should not have.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 6, "Thread changed counter when it should not have.");

//     TEST_ASSERT_TRUE_MESSAGE(not_orphaned_lock(&counter, semaphore) == pdTRUE, "Function acquired semaphore when it should not have.");
//     TEST_ASSERT_TRUE_MESSAGE(counter == 7, "Thread changed counter when it should not have.");
// }

void test_run_thread(void *args)
{
    while(1) {
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_same_priority_busy_busy);
        RUN_TEST(test_same_priority_yield_yield);
        UNITY_END();
        sleep_ms(10000);
    }
}

int main (void)
{
    stdio_init_all();
    sleep_ms(5000); // Give time for TTY to attach.
    xTaskCreate(test_run_thread, "RunTests",
        configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);
    vTaskStartScheduler();
    return 0;

}
