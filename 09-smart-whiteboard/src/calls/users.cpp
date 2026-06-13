#include "users.h"
#include "websocket.h"
#include <ArduinoJson.h>

//Color tables
const char* USER_COLORS_HEX[MAX_USERS] = {
  "#E74C3C",   // red
  "#3498DB",   // blue
  "#2ECC71",   // green
  "#F1C40F",   // yellow
  "#9B59B6",   // purple
  "#E67E22",                                                                                                    
};

//Globals
User              users[MAX_USERS];
int               userCount = 0;
SemaphoreHandle_t userMutex = nullptr;

//Init
void usersInit() {
  memset(users, 0, sizeof(users));
  userCount = 0;
  userMutex = xSemaphoreCreateMutex();
}

//Find
int findUserSlot(int clientId) {
  for (int i = 0; i < MAX_USERS; i++) {
    if (users[i].active && users[i].clientId == clientId) return i;
  }
  return -1;
}

//Add
int addUser(int clientId, const char* name) {
  if (userCount >= MAX_USERS) return -1;

  for (int i = 0; i < MAX_USERS; i++) {
    if (!users[i].active) {
      users[i].clientId = clientId;

      strncpy(users[i].name, name, 31);
      
      users[i].name[31] = '\0';
      users[i].colorIdx = i;
      users[i].active   = true;
      userCount++;
      return i;
    }
  }
  return -1;
}

//Remove
void removeUser(int clientId) {
  int userId = findUserSlot(clientId);
  
  if (userId < 0) return;
  users[userId].active = false;
  userCount--;
}

//Broadcast user list to all clients
void broadcastUserList() {
  StaticJsonDocument<512> doc;
  doc["type"] = "users";
  JsonArray arr = doc.createNestedArray("users");

  for (int i = 0; i < MAX_USERS; i++) {
    if (!users[i].active) continue;
    JsonObject u = arr.createNestedObject();
    u["name"]  = users[i].name;
    u["color"] = USER_COLORS_HEX[users[i].colorIdx];
    u["id"]    = users[i].clientId;
  }

  char buf[512];
  serializeJson(doc, buf);
  ws.textAll(buf);
}