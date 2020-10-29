#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "user.h"
#include <freertos/event_groups.h>

EventGroupHandle_t evtGpHandle;
const int gotTask1 = BIT0;
const int gotTask2 = BIT1;

User user;
xQueueHandle qHandler;
void updateUser(char *name, char *address, int age)
{
    strcpy(user.name, name);
    strcpy(user.address, address);
    user.age = age;
}

void task1(void *params)
{
    while (1)
    {

        printf("%s running ...\n", (char *)params);
        xEventGroupSetBits(evtGpHandle, gotTask1);
        // updateUser("Tailor", "China", 32);
        // setUser(user);
        // User *getuser = getUser();
        // printf("name1: %s, ", getuser->name);
        // printf("addr1: %s, ", getuser->address);
        // printf("age1: %d\n", getuser->age);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
void task2(void *params)
{
    while (1)
    {
        printf("%s running ...\n", (char *)params);
        xEventGroupSetBits(evtGpHandle, gotTask2);
        // updateUser("James", "USA", 26);
        // setUser(user);
        // User *getuser = getUser();
        // printf("name2: %s, ", getuser->name);
        // printf("addr2: %s, ", getuser->address);
        // printf("age2: %d\n", getuser->age);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

void task3(void *params)
{
    while (1)
    {
        xEventGroupWaitBits(evtGpHandle, gotTask1 | gotTask2, true, true, portMAX_DELAY);
        printf("Received some stuff from task 1 or task2! \n");
    }
}

void app_main()
{
    evtGpHandle = xEventGroupCreate();
    xTaskCreate(&task1, "counter 1", 2048, "Task 1", 2, NULL); //passing parameter task 1
    xTaskCreate(&task2, "counter 2", 2048, "Task 2", 2, NULL); //passing parameter task 2
    xTaskCreate(&task3, "counter 3", 2048, "Task 3", 2, NULL); //passing parameter task 3
    // xTaskCreatePinnedToCore(&task2, "counter 2", 2048, "task 2", 2, NULL, 1); //create task in the second core of ESP32  - 2 core capability
}