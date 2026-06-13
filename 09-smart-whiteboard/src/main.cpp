#include <Arduino.h>
#include <WiFi.h>
#include <LittleFS.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

#include "display.h"
#include "./calls/users.h"
#include "./calls/websocket.h"

static const char* AP_SSID = "YOUR_SSID";
static const char* AP_PASS = "YOUR_PASSWORD";

AsyncWebServer server(80);

static void handleToolbarPress(int x) {
  if (x >= BTN_CLEAR_X && x < BTN_CLEAR_X + BTN_CLEAR_W) {
    clearCanvas(); histClear(); ws.textAll("{\"type\":\"clear\"}");
  }
}

static void handleTouch(int x, int y) {
  if (y < TOOLBAR_H) {
    handleToolbarPress(x);
    return;
  }
}

void setup() {
  Serial.begin(115200);
  displayInit();
  usersInit();
  
  if (!LittleFS.begin(true)) return;
  
  WiFi.begin(AP_SSID, AP_PASS);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  
  Serial.println("\nConnected! → http://" + WiFi.localIP().toString());

  ws.onEvent(onWsEvent);
  server.addHandler(&ws);
  server.serveStatic("/", LittleFS, "/").setDefaultFile("index.html");
  server.begin();
  
  histClear();
}

void loop() {
  static int _lastClean = 0;
  if (millis() - _lastClean > 500) { ws.cleanupClients(); _lastClean = millis(); }

  wsQueueTick();

  int x, y;
  if (lcd.getTouch(&x, &y)) {
    handleTouch(x, y);
  }
  
  delay(10); 
}