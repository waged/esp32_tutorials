
#include "user.h"
#include <string.h>


static User _user; 

User *getUser(){
    return &_user;
}

void setUser(User user){
    strcpy(_user.name , user.name);
    strcpy(_user.address , user.address);
    _user.age = user.age;
}