#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "user.h"

void updateUser(User *user)
{
    strcpy(user->name, "Tailor");
    strcpy(user->address, "China");
    user->age = 35;
}

void task1(void* params)
{
    int counter = 0;
    User user;
    updateUser(&user);
    setUser(user);
    User *getuser = getUser();
    printf("\n\n user_name: %s\n", getuser->name);
    printf("\n user_address: %s\n", getuser->address);
    printf("\n user_age: %d\n", getuser->age);
    while (1)
    {
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        printf("%s counter now equals %d!\n", (char *)params ,++counter);
    }
}
void task2(void* params) // never executed!!
{
    int counter = 0;
    while (1)
    {
        vTaskDelay(2000 / portTICK_PERIOD_MS);
        printf("%s counter now equals %d!\n",(char *)params ,++counter);
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

    xTaskCreate(&task1, "counter 1", 2048, "task 1", 2, NULL); //passing parameter task 1
    xTaskCreate(&task2, "counter 2", 2048, "task 2", 2, NULL); //passing parameter task 2
}