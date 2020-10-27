#ifndef _USER_H
#define _USER_H

typedef struct User
{
    char name[20];
    char address[20];
    int age;
} User;

User *getUser();

void setUser(User user);

#endif