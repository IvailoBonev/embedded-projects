#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>

//Config
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";

//RGB LED pins
#define PIN_R 27
#define PIN_G 26
#define PIN_B 25

//PWM settings
#define PWM_FREQ 5000 // 5 kHz PWM frequency
#define PWM_RES 8 // 8-bit: values 0–255
#define CH_R 0 // channel for red
#define CH_G 1 // channel for green
#define CH_B 2 // channel for blue

WebServer server(80);

//Set LED Colors
void setColor(int r, int g, int b) {
  ledcWrite(CH_R, r);
  ledcWrite(CH_G, g);
  ledcWrite(CH_B, b);
}

//Open files logic
void serveFile(const char* path, const char* contentType) {
  File file = SPIFFS.open(path, "r");

  if (!file) {
    server.send(404, "text/plain", "File not found");
    return;
  }

  server.streamFile(file, contentType);
  file.close();
}

//Route

// GET /
void handleRoot() {
  if (server.hasArg("r") && server.hasArg("g") && server.hasArg("b")) {
    int r = server.arg("r").toInt();
    int g = server.arg("g").toInt();
    int b = server.arg("b").toInt();
    setColor(r, g, b);

    Serial.printf("Color → R:%d G:%d B:%d\n", r, g, b);
    
    server.send(200, "text/plain", "OK");
    return;
  }
  serveFile("/index.html", "text/html");
}

// GET /style.css
void handleCSS() {
  serveFile("/style.css", "text/css");
}

// GET /script.js
void handleJS() {
  serveFile("/script.js", "application/javascript");
}

// ── Setup ───────────────────────────────────────────────
void setup() {
  Serial.begin(115200);
  SPIFFS.begin(true);

  // PWM setup
  ledcSetup(CH_R, PWM_FREQ, PWM_RES);
  ledcSetup(CH_G, PWM_FREQ, PWM_RES);
  ledcSetup(CH_B, PWM_FREQ, PWM_RES);

  ledcAttachPin(PIN_R, CH_R);
  ledcAttachPin(PIN_G, CH_G);
  ledcAttachPin(PIN_B, CH_B);

  setColor(0, 50, 125);   // default: blue

  // WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected! → http://" + WiFi.localIP().toString());

  // Routes
  server.on("/",          handleRoot);
  server.on("/style.css", handleCSS);
  server.on("/script.js", handleJS);

  server.begin();
  Serial.println("Server started.");
}

void loop() {
  server.handleClient();
}