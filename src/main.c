#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "user.h"

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
    int count = 0;
    while (1)
    {
        count++;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        long ok = xQueueSend(qHandler, &count, 1000 / portTICK_PERIOD_MS);
        if (ok)
        {
            printf("\n%s added msg to queue \n", (char *)params);
            updateUser("Tailor", "China", 32);
            setUser(user);
            User *getuser = getUser();
            printf("name1: %s, ", getuser->name);
            printf("addr1: %s, ", getuser->address);
            printf("age1: %d\n", getuser->age);
        }
        else
        {
            printf("%s :Failed to add message to Queue\n", (char *)params);
        }
    }
}
void task2(void *params)
{
    while (1)
    {
        int rxCount;
        if (xQueueReceive(qHandler, &rxCount, 5000 / portTICK_PERIOD_MS))
        {
        printf("\n%s running and got number %d \n", (char *)params, rxCount);
        updateUser("James", "USA", 26);
        setUser(user);
        User *getuser = getUser();
        printf("name2: %s, ", getuser->name);
        printf("addr2: %s, ", getuser->address);
        printf("age2: %d\n", getuser->age);
        }
      
    }
}

void app_main()
{
    qHandler = xQueueCreate(3, sizeof(int));
    xTaskCreate(&task1, "counter 1", 2048, "Task 1", 2, NULL); //passing parameter task 1
    xTaskCreate(&task2, "counter 2", 2048, "Task 2", 2, NULL); //passing parameter task 2
    // xTaskCreatePinnedToCore(&task2, "counter 2", 2048, "task 2", 2, NULL, 1); //create task in the second core of ESP32  - 2 core capability
}