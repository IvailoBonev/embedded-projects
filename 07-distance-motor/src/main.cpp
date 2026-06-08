#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306   oled(128, 64, &Wire, -1);

// Motor (L293D chip)
const int CONTROL_PIN_1    = 2;
const int CONTROL_PIN_2    = 3;
const int ENABLE_PIN       = 9;

//Switches
const int DIR_SWITCH_PIN   = 4; //direction
const int ONOFF_SWITCH_PIN = 5;

//HC-SR04 Ultrasonic sensor
const int TRIG_PIN         = 6;
const int ECHO_PIN         = 7;

//Alert
const int BUZZER_PIN       = 10;
const int LED_PIN          = 13;

const int DIST_FULL_SPEED  = 32;   // above this → full speed
const int DIST_DANGER      = 5;    // below this → stop + alert

//Motor characteristics
bool motorEnabled  = false;
bool motorForward  = true;

const unsigned long DEBOUNCE_MS = 50;

//returns distance in cm, or -1 if out of range
int readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 15000); // 15ms timeout → ~2.5m max range
  
  if (duration == 0) return -1;
  return duration / 58;
}


// Maps distance to motor speed
int distanceToSpeed(int cm) {
  if (cm < 0) return 0;
  if (cm <= DIST_DANGER) return 0;
  if (cm >= DIST_FULL_SPEED) return 255;

  return constrain(map(cm, DIST_DANGER, DIST_FULL_SPEED, 70, 255),110, 255);
}

//Set motor direction
void setMotorDirection(bool forward) {
  analogWrite(ENABLE_PIN, 0); // disable motor before changing direction to prevent shoot-through
  delay(10);

  digitalWrite(CONTROL_PIN_1, forward ? HIGH : LOW);
  digitalWrite(CONTROL_PIN_2, forward ? LOW  : HIGH);
}

//Debounced rising edge detection for switches
bool risingEdge(int pin, bool &lastState, unsigned long &lastTime) {
  bool reading = digitalRead(pin);

  if (reading != lastState && (millis() - lastTime) > DEBOUNCE_MS) {
    lastTime  = millis();
    lastState = reading;

    return (reading == HIGH);
  }

  return false;
}

//OLED Bar
void drawSpeedBar(int spd) {
  const int BAR_X = 36;
  const int BAR_Y = 45;
  const int BAR_W = 88;
  const int BAR_H = 10;

  oled.drawRect(BAR_X, BAR_Y, BAR_W, BAR_H, WHITE);

  int fill = map(spd, 103, 255, 0, BAR_W - 2);
  if (fill > 0)
    oled.fillRect(BAR_X + 1, BAR_Y + 1, fill, BAR_H - 2, WHITE);
}

//OLED
void updateOLED(int distance, int speed, bool danger) {
  oled.clearDisplay();

  //Row 0: Distance (big)
  oled.setTextSize(1);
  oled.setTextColor(WHITE);
  oled.setCursor(0, 0);
  oled.print("DIST");

  oled.setTextSize(2);
  oled.setCursor(36, 0);

  if (distance < 0) {
    oled.print("---");
  } else {
    oled.print(" "); 
    oled.print(distance);
  }

  oled.setTextSize(1);
  oled.setCursor(96, 8);
  oled.print("cm");

  //Row 1: Speed bar
  oled.setCursor(0, 47);
  oled.print("SPD");
  drawSpeedBar(speed);

  //Row 2: Direction | Motor state | Danger
  oled.setCursor(0, 57);
  oled.print(motorForward ? "FWD" : "REV");

  oled.setCursor(30, 57);
  oled.print(motorEnabled ? "ON " : "OFF");

  if (danger) {
    oled.setCursor(60, 57);
    oled.print("!! STOP !!");
  }

  //launch update
  oled.display();
}

void setup() {
  Serial.begin(9600);
  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);

  pinMode(CONTROL_PIN_1, OUTPUT);     //motor direction control pin 1
  pinMode(CONTROL_PIN_2, OUTPUT);     //motor direction control pin 2
  pinMode(ENABLE_PIN, OUTPUT);        //motor speed control (PWM)
  pinMode(TRIG_PIN, OUTPUT);          //ultrasonic sensor trigger pin
  pinMode(ECHO_PIN, INPUT);           //ultrasonic sensor echo pin
  pinMode(BUZZER_PIN, OUTPUT); 
  pinMode(LED_PIN, OUTPUT);
  pinMode(DIR_SWITCH_PIN, INPUT);     //direction toggle switch
  pinMode(ONOFF_SWITCH_PIN, INPUT);   //on/off toggle switch

  setMotorDirection(motorForward);
  analogWrite(ENABLE_PIN, 0);
}

void loop() {
  //Debounced switch states
  static bool onOffState = LOW, dirState = LOW;
  static unsigned long onOffTime = 0, dirTime = 0;

  if (risingEdge(ONOFF_SWITCH_PIN, onOffState, onOffTime)) {
    motorEnabled = !motorEnabled;
  }

  if (risingEdge(DIR_SWITCH_PIN, dirState, dirTime)) {
    motorForward = !motorForward;

    setMotorDirection(motorForward);
  }

  //Sensor read
  int distance = readDistanceCM();
  int speed    = distanceToSpeed(distance);

  //Alert (buzzer + LED)
  bool danger = (distance >= 0 && distance <= DIST_DANGER);
  analogWrite(BUZZER_PIN, danger ? 28 : 0);
  digitalWrite(LED_PIN, danger ? HIGH : LOW);

  //Drive motor
  analogWrite(ENABLE_PIN, (motorEnabled && !danger) ? speed : 0);

  updateOLED(distance, speed, danger);

  //Serial monitor
  Serial.print("Distance: ");
  if (distance < 0) Serial.print("out of range");
  else { Serial.print(distance); Serial.print(" cm"); }
  Serial.print("  |  Speed: ");
  Serial.print(speed);
  Serial.print("/255  |  Motor: ");
  Serial.print(motorEnabled ? "ON" : "OFF");
  Serial.print("  |  Dir: ");
  Serial.print(motorForward ? "FWD" : "REV");
  Serial.print("  |  Alert: ");
  Serial.println(danger ? "DANGER" : "OK");

  delay(60);
}