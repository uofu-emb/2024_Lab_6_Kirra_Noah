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

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("Thread thread B has run for %lld ticks.\n", b_run_time);

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

    printf("Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("Thread thread B has run for %lld ticks.\n", b_run_time);

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

    printf("Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("Thread thread B has run for %lld ticks.\n", b_run_time);

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_diff_priority_busy_busy_high_first(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_busy, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_a);

    vTaskDelay(200);

    xTaskCreate(busy_busy, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("Thread thread B has run for %lld ticks.\n", b_run_time);

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_diff_priority_busy_busy_low_first(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_busy, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &task_a);

    vTaskDelay(200);

    xTaskCreate(busy_busy, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("Thread thread B has run for %lld ticks.\n", b_run_time);

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_diff_priority_yield_yield(void)
{
    TaskHandle_t task_a, task_b;
    configRUN_TIME_COUNTER_TYPE a_run_time, b_run_time;

    xTaskCreate(busy_yield, "Thread A",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_MEDIUM_PRIORITY, &task_a);
    xTaskCreate(busy_yield, "Thread B",
                TEST_THREAD_STACK_SIZE, NULL, TEST_THREAD_LOW_PRIORITY, &task_b);

    vTaskDelay(5000);

    a_run_time = ulTaskGetRunTimeCounter(task_a);
    b_run_time = ulTaskGetRunTimeCounter(task_b);

    printf("Thread thread A has run for %lld ticks.\n", a_run_time);
    printf("Thread thread B has run for %lld ticks.\n", b_run_time);

    vTaskDelete(task_a);
    vTaskDelete(task_b);
}

void test_run_thread(void *args)
{
    while(1) {
        printf("Start tests\n");
        UNITY_BEGIN();
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
    sleep_ms(5000); // Give time for TTY to attach.
    xTaskCreate(test_run_thread, "RunTests",
        configMINIMAL_STACK_SIZE, NULL, TEST_RUNNER_PRIORITY, NULL);
    vTaskStartScheduler();
    return 0;

}
