#pragma once
#include <Arduino.h>
#include <freertos/semphr.h>

static const int MAX_USERS = 6;
extern const char* USER_COLORS_HEX[6];

struct User {
  int clientId;
  char name[32];
  int colorIdx;
  bool active;
};

extern User users[MAX_USERS];
extern int userCount;
extern SemaphoreHandle_t userMutex;

void usersInit();
int  findUserSlot(int clientId);
int  addUser(int clientId, const char* name);
void removeUser(int clientId);
void broadcastUserList();