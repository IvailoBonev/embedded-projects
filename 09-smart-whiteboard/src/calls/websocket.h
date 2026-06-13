#pragma once
#include <ESPAsyncWebServer.h>

// ─── Global WebSocket instance (used by users.cpp + touch handler) ────────────
extern AsyncWebSocket ws;

// ─── Event handler — pass to ws.onEvent() in setup() ─────────────────────────
void onWsEvent(AsyncWebSocket* server, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len);

void wsQueueTick();

void histPush(const char* s);
void histReplay(AsyncWebSocketClient* client);
void histClear();