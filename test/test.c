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

#define TEST_RUNNER_PRIORITY        (tskIDLE_PRIORITY + 5UL)
#define TEST_THREAD_HIGH_PRIORITY   (TEST_RUNNER_PRIORITY - 1UL)
#define TEST_THREAD_MEDIUM_PRIORITY (TEST_RUNNER_PRIORITY - 2UL)
#define TEST_THREAD_LOW_PRIORITY    (TEST_RUNNER_PRIORITY - 3UL)
#define TEST_THREAD_STACK_SIZE      (configMINIMAL_STACK_SIZE)

void setUp(void) {}

void tearDown(void) {}

SemaphoreHandle_t semaphore;
void high_priority_task(__unused void *params) {
    printf("    Hello from high priority thread.\n");
    vTaskDelay(200);
    while(1){
        printf("    Trying to take semaphore in high priority thread.\n");
        bool semaphore_pass = xSemaphoreTake(semaphore, portMAX_DELAY);
        if(semaphore_pass){
            printf("    We have the semaphore in high priority!\n");
            for(int i = 0; i < 100000000; i++) {
                if (i % 10000000 == 0);
            }
            xSemaphoreGive(semaphore);
        }
    }
}

void medium_priority_task(__unused void *params) {
    printf("    Hello from medium priority thread.\n");
    vTaskDelay(100);
    while(1){
        for(int i = 0; i < 100000000; i++) {
            if (i % 10000000 == 0) {
                printf("    Medium priority thread: counted to %d.\n", i);
            }
        }
    }
}

void low_priority_task(__unused void *params) {
    printf("    Hello from low priority thread.\n");
    while(1){
        printf("    Trying to take semaphore in low priority thread.\n");
        bool semaphore_pass = xSemaphoreTake(semaphore, 500);
        if(semaphore_pass){
            printf("    We have the semaphore in low priority!\n");
            for(int i = 0; i < 100000000; i++) {
                if (i % 10000000 == 0);
            }
            xSemaphoreGive(semaphore);
        }
    }
}

void test_priority_inversion_binary(void)
{
    TaskHandle_t hp_task, mp_task, lp_task;
    configRUN_TIME_COUNTER_TYPE low_run_time, medium_run_time, high_run_time;
    semaphore = xSemaphoreCreateBinary();
    xSemaphoreGive(semaphore);

    xTaskCreate(low_priority_task, "Low Priority Thread",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &lp_task);
    xTaskCreate(medium_priority_task, "Medium Priority Thread",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &mp_task);
    xTaskCreate(high_priority_task, "High Priority Thread",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_HIGH_PRIORITY, &hp_task);
    
    vTaskDelay(5000);

    low_run_time = ulTaskGetRunTimeCounter(lp_task);
    medium_run_time = ulTaskGetRunTimeCounter(mp_task);
    high_run_time = ulTaskGetRunTimeCounter(hp_task);

    printf("  Low priority thread has run for %lld ticks.\n", low_run_time);
    printf("  Medium priority thread has run for %lld ticks.\n", medium_run_time);
    printf("  High priority thread has run for %lld ticks.\n", high_run_time);

    TEST_ASSERT_MESSAGE(high_run_time < medium_run_time, "High-priority thread did not have its priority inverted.");

    vTaskDelete(hp_task);
    vTaskDelete(mp_task);
    vTaskDelete(lp_task);
}

void test_priority_inversion_mutex(void)
{
    TaskHandle_t hp_task, mp_task, lp_task;
    configRUN_TIME_COUNTER_TYPE low_run_time, medium_run_time, high_run_time;
    // Using a mutex instead of a binary solves priority inversion 
    //  because it implements priority inheritence.
    semaphore = xSemaphoreCreateMutex();
    xSemaphoreGive(semaphore);

    xTaskCreate(low_priority_task, "Low Priority Thread",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &lp_task);
    xTaskCreate(medium_priority_task, "Medium Priority Thread",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &mp_task);
    xTaskCreate(high_priority_task, "High Priority Thread",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_HIGH_PRIORITY, &hp_task);
    
    vTaskDelay(5000);

    low_run_time = ulTaskGetRunTimeCounter(lp_task);
    medium_run_time = ulTaskGetRunTimeCounter(mp_task);
    high_run_time = ulTaskGetRunTimeCounter(hp_task);

    printf("  Low priority thread has run for %lld ticks.\n", low_run_time);
    printf("  Medium priority thread has run for %lld ticks.\n", medium_run_time);
    printf("  High priority thread has run for %lld ticks.\n", high_run_time);

    TEST_ASSERT_MESSAGE(high_run_time > medium_run_time, "Priority was inverted; priority inheritance did not work.");

    vTaskDelete(hp_task);
    vTaskDelete(mp_task);
    vTaskDelete(lp_task);
}

void test_same_priority_busy_busy(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_busy, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_a);
    xTaskCreate(busy_busy, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("  Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("  Thread thread B has run for %lld ticks.\n", b_run_time);

    TEST_ASSERT_MESSAGE(a_run_time > 2250000, "Thread A was starved.");
    TEST_ASSERT_MESSAGE(b_run_time > 2250000, "Thread B was starved.");

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_same_priority_yield_yield(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_yield, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_a);
    xTaskCreate(busy_yield, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("  Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("  Thread thread B has run for %lld ticks.\n", b_run_time);

    TEST_ASSERT_MESSAGE(a_run_time > 2250000, "Thread A was starved.");
    TEST_ASSERT_MESSAGE(b_run_time > 2250000, "Thread B was starved.");

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_same_priority_yield_busy(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_busy, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_a);
    xTaskCreate(busy_yield, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("  Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("  Thread thread B has run for %lld ticks.\n", b_run_time);

    
    TEST_ASSERT_MESSAGE(b_run_time < a_run_time, "Yield thread did not yield.");
    TEST_ASSERT_MESSAGE(a_run_time > 4500000, "Busy thread did not run enough.");

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_diff_priority_busy_busy_high_first(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_busy, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_HIGH_PRIORITY, &task_a);

    vTaskDelay(10);

    xTaskCreate(busy_busy, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("  Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("  Thread thread B has run for %lld ticks.\n", b_run_time);

    TEST_ASSERT_MESSAGE(a_run_time > b_run_time, "High priority thread did not run enough.");
    TEST_ASSERT_MESSAGE(b_run_time < 50, "Low priority thread ran when it should not have been preempted.");

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_diff_priority_busy_busy_low_first(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_busy, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &task_a);

    vTaskDelay(10);

    xTaskCreate(busy_busy, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_HIGH_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("  Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("  Thread thread B has run for %lld ticks.\n", b_run_time);

    TEST_ASSERT_MESSAGE(b_run_time > a_run_time, "High priority thread did not run enough.");
    TEST_ASSERT_MESSAGE(a_run_time < 20000, "Low priority thread ran for too long.");

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_diff_priority_yield_yield(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_yield, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_HIGH_PRIORITY, &task_a);
    xTaskCreate(busy_yield, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("  Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("  Thread thread B has run for %lld ticks.\n", b_run_time);

    TEST_ASSERT_MESSAGE(a_run_time > b_run_time, "High priority thread did not run enough.");
    TEST_ASSERT_MESSAGE(b_run_time < 50, "Low priority thread ran when it should not have been preempted.");

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_run_thread(void *args)
{
    while(1) {
        printf("Start tests\n");
        UNITY_BEGIN();
        RUN_TEST(test_priority_inversion_binary);
        RUN_TEST(test_priority_inversion_mutex);
        RUN_TEST(test_same_priority_busy_busy);
        RUN_TEST(test_same_priority_yield_yield);
        RUN_TEST(test_same_priority_yield_busy);
        RUN_TEST(test_diff_priority_busy_busy_high_first);
        RUN_TEST(test_diff_priority_busy_busy_low_first);
        RUN_TEST(test_diff_priority_yield_yield);
        UNITY_END();
        sleep_ms(10000);
    }
}

int main (void)
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    sleep_ms(5000); // Give time for TTY to attach.
    xTaskCreate(test_run_thread, "RunTests",
        configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);
    vTaskStartScheduler();
    return 0;

}
