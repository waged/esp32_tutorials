#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include "user.h"

User user;
xSemaphoreHandle binSemaphorHandle;

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
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("\n%s running \n", (char *)params);
        updateUser("Tailor", "China", 32);
        setUser(user);
        User *getuser = getUser();
        printf("name1: %s, ", getuser->name);
        printf("addr1: %s, ", getuser->address);
        printf("age1: %d\n", getuser->age);
        xSemaphoreGive(binSemaphorHandle);
    }
}

void task2(void *params)
{
    while (1)
    {
        xSemaphoreTake(binSemaphorHandle, portMAX_DELAY);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        printf("\n%s running. \n", (char *)params);
        updateUser("James", "USA", 26);
        setUser(user);
        User *getuser = getUser();
        printf("name2: %s, ", getuser->name);
        printf("addr2: %s, ", getuser->address);
        printf("age2: %d\n", getuser->age);
 
    }
}

void app_main()
{
    //FreeRTOS example to show Multitasking

    //friendly name of the task
    // how much memory you want to keep it for this task
    // send parameters NULL for no passing
    // piriority 1 lowest  5 highest
    // update modify delete this task
    // mutexHandle = xSemaphoreCreateMutex();  //timeout relaese
    binSemaphorHandle = xSemaphoreCreateBinary();              //logic  release
    xTaskCreate(&task1, "counter 1", 2048, "Task 1", 2, NULL); //passing parameter task 1
    xTaskCreate(&task2, "counter 2", 2048, "Task 2", 2, NULL); //passing parameter task 2
    // xTaskCreatePinnedToCore(&task2, "counter 2", 2048, "task 2", 2, NULL, 1); //create task in the second core of ESP32  - 2 core capability

    /*
    FreeRTOS interaction between 2 or more tasks
                        I want to communicate directly to another task.care about counting or sending simple data --YES--> Task Notification
                        || NO
                        Someone needs to block or unblock a task, I don't care who      -- Yes--> Semaphore 
                        || NO
                        Only the task that blocks another task can unblock it           --YES-->   Mutex    
                        || NO
                        I need to pass data from one task to another                    --YES-->   Queue
                        || NO
                        Mulltiple things need to happen before I unblock a task         --YES--> Event Group
    */
}