#include "websocket.h"
#include "users.h"
#include "display.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

AsyncWebSocket ws("/ws");

static const int PAYLOAD_MAX_LEN      = 1024;  // max bytes for a single WS JSON payload
const int MIN_VALID_JSON_LEN = 5;
static const int DRAW_QUEUE_CAPACITY  = 8;      // max pending draw commands before oldest is dropped

//LittleFS History
void histPush(const char* s) {
    File f = LittleFS.open("/history.jsonl", "a"); 
    if (f) {
        f.println(s);
        f.close();
    }
}

void histClear() {
    if (LittleFS.exists("/history.jsonl")) {
        LittleFS.remove("/history.jsonl");
    }
    Serial.println("[FS] History Cleared");
}

void histReplay(AsyncWebSocketClient* client) {
    if (!LittleFS.exists("/history.jsonl")) return;

    File f = LittleFS.open("/history.jsonl", "r");
    if (!f) return;

    while (f.available()) {
        String line = f.readStringUntil('\n');
        if (line.length() > MIN_VALID_JSON_LEN) {
            client->text(line);
            yield();
            delay(10); 
        }
    }
    f.close();
}

// Ring buffer: draw commands received over WebSocket are queued here
// and consumed one per loop() tick by wsQueueTick().
// head  → where the next write goes
// tail  → where the next read comes from
// count → how many entries are currently stored
static char drawBuffer[DRAW_QUEUE_CAPACITY][PAYLOAD_MAX_LEN];
static int  drawHead  = 0;
static int  drawTail  = 0;
static int  drawCount = 0;

static void qPush(const char* s) {
    if (drawCount >= DRAW_QUEUE_CAPACITY) { 
        drawTail = ( drawTail + 1 ) % DRAW_QUEUE_CAPACITY; 
        drawCount--; 
    }

    strncpy(drawBuffer[drawHead], s, PAYLOAD_MAX_LEN - 1);
    drawBuffer[drawHead][PAYLOAD_MAX_LEN-1] = '\0';

    drawHead = (drawHead + 1) % DRAW_QUEUE_CAPACITY;
    drawCount++;
}

static bool qPop(char* out) {
    if (drawCount == 0) return false;

    strncpy(out, drawBuffer[drawTail], PAYLOAD_MAX_LEN);

    drawTail = (drawTail + 1) % DRAW_QUEUE_CAPACITY;
    drawCount--;
    return true;
}

void wsQueueTick() {
    static char buf[PAYLOAD_MAX_LEN];
    if (!qPop(buf)) return;

    DynamicJsonDocument doc(PAYLOAD_MAX_LEN + 512);
    if (deserializeJson(doc, buf) != DeserializationError::Ok) return;

    const char* msgType = doc["type"];
    if (!msgType) return;
    
    int col = hexToColor565(doc["color"] | "#000000");

    if (!strcmp(msgType, "stroke")) {
        JsonArray pts = doc["points"];
        int sz = doc["size"] | 4;
        bool erase = doc["erase"] | false;
        
        if (pts.size() == 1) {
          renderDraw(pts[0][0], pts[0][1], pts[0][0], pts[0][1], col, sz);
        } else {
            int px = pts[0][0], py = pts[0][1];

            for (int i=1; i < pts.size(); i++) {
                int x = pts[i][0], y = pts[i][1];

                if (erase) renderErase(x, y, px, py);
                else renderDraw(x, y, px, py, col, sz);

                px = x; py = y;
            }
        }
    } 
    else if (!strcmp(msgType, "text")) {
        renderText(doc["x"] | 0, doc["y"] | 0, doc["text"] | "", col);
    } 
    else if (!strcmp(msgType, "shape")) {
        renderShape(doc["shape"]|"line", doc["x1"]|0, doc["y1"]|0, doc["x2"]|0, doc["y2"]|0, col);
    } 
    else if (!strcmp(msgType, "clear")) {
        clearCanvas();
    }
}

void onWsEvent(AsyncWebSocket* srv, AsyncWebSocketClient* client,
               AwsEventType type, void* arg, uint8_t* data, size_t len) {
    
    if (type == WS_EVT_DISCONNECT) {
        xSemaphoreTake(userMutex, portMAX_DELAY);
        removeUser(client->id());
        xSemaphoreGive(userMutex);

        broadcastUserList();
    }
    else if (type == WS_EVT_DATA) {
        AwsFrameInfo* info = (AwsFrameInfo*)arg;

        if (!info->final || info->index != 0 || info->len != len) return;

        data[len] = '\0';

        DynamicJsonDocument doc(PAYLOAD_MAX_LEN + 512);
        if (deserializeJson(doc, (char*)data)) return;

        const char* msgType = doc["type"];
        if (!msgType) return;

        if (!strcmp(msgType, "join")) {
            xSemaphoreTake(userMutex, portMAX_DELAY);
            int userId = addUser(client->id(), doc["name"] | "anon");
            xSemaphoreGive(userMutex);

            if (userId < 0) {
                client->text("{\"type\":\"reject\",\"reason\":\"Session full (max 6 players)\"}");
                return;
            }
            
            StaticJsonDocument<128> ack;
            ack["type"] = "joined";
            ack["color"] = USER_COLORS_HEX[users[userId].colorIdx];

            String s; serializeJson(ack, s);
            client->text(s);

            histReplay(client); 
            broadcastUserList();
        } 
        else if (!strcmp(msgType, "chat")) {
            xSemaphoreTake(userMutex, portMAX_DELAY);
            int userId = findUserSlot(client->id());
            xSemaphoreGive(userMutex);

            if (userId >= 0) {
                doc["name"] = users[userId].name;
                doc["color"] = USER_COLORS_HEX[users[userId].colorIdx];

                String out; serializeJson(doc, out);
                ws.textAll(out);
                histPush(out.c_str());
            }
        }
        else if (!strcmp(msgType, "stroke") || !strcmp(msgType, "shape") || !strcmp(msgType, "text") || !strcmp(msgType, "clear")) {
            xSemaphoreTake(userMutex, portMAX_DELAY);
            int userId = findUserSlot(client->id());
            xSemaphoreGive(userMutex);

            if (userId >= 0 && !doc.containsKey("color")) {
                doc["color"] = USER_COLORS_HEX[users[userId].colorIdx];
            }
            
            String out; 
            serializeJson(doc, out);
            ws.textAll(out);
            
            if (strcmp(msgType, "clear") == 0) {
                histClear();
            } else {
                histPush(out.c_str()); 
            }
            qPush(out.c_str()); 
        }
    }
}