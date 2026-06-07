#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306   oled(128, 64, &Wire, -1);
LiquidCrystal_I2C  lcd(0x27, 16, 2);

const int SWITCH_PIN       = A3;
const int LED_PINS[]       = {2, 3, 4, 5, 6, 7};
const int LED_COUNT        = 6;

//Timing 
const unsigned long TILT_INTERVAL_MS   = 6000;
const unsigned long SHOOT_FRAME_MS     = 30;
const unsigned long LCD_BLINK_PERIOD   = 500;

//Star field 
const int NUM_STARS        = 35;
const int STARS_PER_LED    = 3;

int starX[NUM_STARS];
int starY[NUM_STARS];

//Shooting star
const float SHOOT_DX = 3.0f;
const float SHOOT_DY = 1.5f;

bool  shootingActive  = false;
float shootX          = 0;
float shootY          = 0;
unsigned long lastShootFrame = 0;

//State
int           ledsOn   = 0;
unsigned long lastTilt = 0;

//Star field functions
void initStarField() {
  for (int i = 0; i < NUM_STARS; i++) {
    starX[i] = random(2, 126);
    starY[i] = random(18, 62);
  }
}

//draw a star
void drawStar(int x, int y) {
  oled.drawPixel(x, y, SSD1306_WHITE);
  oled.drawPixel(x + 1, y, SSD1306_WHITE);
  oled.drawPixel(x - 1, y, SSD1306_WHITE);
  oled.drawPixel(x, y + 1, SSD1306_WHITE);
  oled.drawPixel(x, y - 1, SSD1306_WHITE);
}

void handleShootingStar() {
  if (!shootingActive) return;

  if (millis() - lastShootFrame < SHOOT_FRAME_MS) return;
  shootX += SHOOT_DX;
  shootY += SHOOT_DY;
  lastShootFrame = millis();
  if (shootX > 128 || shootY > 63) { shootingActive = false; return; }
  
  int x = (int)shootX, y = (int)shootY;
  oled.drawPixel(x, y, SSD1306_WHITE);
  oled.drawPixel(x - 1, y, SSD1306_WHITE);
  oled.drawPixel(x - 2, y - 1, SSD1306_WHITE);
  oled.drawPixel(x - 3, y - 1, SSD1306_WHITE);
}

//turns off all LEDs
void resetLEDs() {
  for (int i = 0; i < LED_COUNT; i++) digitalWrite(LED_PINS[i], LOW);
}


//OLED
void updateOLED() {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(22, 4);
  oled.print("* NIGHT SKY *");

  int starsToShow = ledsOn * STARS_PER_LED;
  for (int i = 0; i < starsToShow; i++) {
    if (random(0, 10) > 1) drawStar(starX[i], starY[i]);
  }

  handleShootingStar();
  oled.display();
}


//LCD
void updateLCD() {
  bool blink   = (millis() / LCD_BLINK_PERIOD) % 2 == 0;
  bool canTilt = millis() - lastTilt > TILT_INTERVAL_MS;

  lcd.setCursor(0, 0);
  lcd.print("LEDs: "); lcd.print(ledsOn); lcd.print(" of 6  ");

  lcd.setCursor(0, 1);
  if (canTilt) {
    lcd.print(blink ? "Tilt!          " : "Status: READY   ");
  } else if (blink) {
    lcd.print("Status: WAIT    ");
  } else {
    int secondsLeft = (TILT_INTERVAL_MS - (millis() - lastTilt)) / 1000 + 1;
    lcd.print("Next in: "); lcd.print(secondsLeft); lcd.print("s      ");
  }
}


//Tilt
void handleTilt() {
  if (digitalRead(SWITCH_PIN) != LOW) return;
  if (millis() - lastTilt <= TILT_INTERVAL_MS) return;

  if (ledsOn < LED_COUNT) {
    digitalWrite(LED_PINS[ledsOn], HIGH); 
  }

  lastTilt = millis();
  if (++ledsOn > LED_COUNT) {
    resetLEDs();

    shootX = 0;
    shootY = random(18, 40);
    shootingActive = true; 
    initStarField();

    ledsOn = 0;
  }
}

void setup() {
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("-- Ready --");

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(SWITCH_PIN, INPUT_PULLUP);
  for (int i = 0; i < LED_COUNT; i++) pinMode(LED_PINS[i], OUTPUT);
  randomSeed(analogRead(A0));

  initStarField();
}

void loop() {
  updateLCD();
  handleTilt();
  updateOLED();
}