#include <Arduino.h>
#include <LiquidCrystal.h>
//RGB LED, photoresistors, LCD display

LiquidCrystal lcd(2, 3, 4, 5, 6, 7);

const int redPin         = 11;
const int greenPin       = 10;
const int bluePin        = 9;

//photoresistors connected to A0, A1, A2
const int redSensorPin   = A0;
const int greenSensorPin = A1;
const int blueSensorPin  = A2;

const int buttonPin      = 12;
const int potPin         = A5;

unsigned long lastLCD    = 0;
unsigned long lastButton = 0;

int redValue   = 0;
int blueValue  = 0;
int greenValue = 0;
bool frozen    = false;

//Accent logic
void applyAccent(int pinValue, int pin, int pin2Value, int pin2, int pin3Value, int pin3) {
  int potValue = analogRead(potPin);

  //check deadzone and dark mode
  if (potValue > 30 && !(redValue < 50 && blueValue < 50 && greenValue < 50)) {
    float s = potValue / 1023.0; //strength
    analogWrite(pin,  constrain((int)(pinValue  + (255 - pinValue)  * s), 0, 255)); //the one with accent
    analogWrite(pin2, constrain((int)(pin2Value * (1.0 - s)),              0, 255));
    analogWrite(pin3, constrain((int)(pin3Value * (1.0 - s)),              0, 255));
  } else {
    analogWrite(pin,  pinValue);
    analogWrite(pin2, pin2Value);
    analogWrite(pin3, pin3Value);
  }
}

void setup() {
  Serial.begin(9600);
  lcd.begin(16, 2);

  pinMode(buttonPin, INPUT);
  pinMode(redPin,    OUTPUT);
  pinMode(bluePin,   OUTPUT);
  pinMode(greenPin,  OUTPUT);
  
  lcd.print("Loading...");
}

void loop() {
  // button with debounce
  if (digitalRead(buttonPin) == HIGH && millis() - lastButton > 600) {
    frozen     = !frozen;
    lastButton = millis();
  }

  // only read sensors when not frozen
  if (!frozen) {
    redValue   = constrain(map(analogRead(redSensorPin),   0, 1023, 0, 255), 0, 255);
    greenValue = constrain(map(analogRead(greenSensorPin), 0, 1023, 0, 255), 0, 255);
    blueValue  = constrain(map(analogRead(blueSensorPin),  0, 1023, 0, 255), 0, 255);
  }

  // write to LED
  if (frozen) {
    //accent if frozen and one color is dominant, otherwise just write the values
    if  (redValue   > greenValue && redValue   > blueValue)
      applyAccent(redValue, redPin, greenValue, greenPin, blueValue, bluePin);
    else if (greenValue > redValue   && greenValue > blueValue)
      applyAccent(greenValue, greenPin, redValue, redPin, blueValue, bluePin);
    else if (blueValue  > redValue   && blueValue  > greenValue)
      applyAccent(blueValue, bluePin, redValue, redPin, greenValue, greenPin);
    else {
      analogWrite(redPin,   redValue);
      analogWrite(greenPin, greenValue);
      analogWrite(bluePin,  blueValue);
    }
  } 
  else {
    analogWrite(redPin,   redValue);
    analogWrite(greenPin, greenValue);
    analogWrite(bluePin,  blueValue);
  }

  // LCD update
  if (millis() - lastLCD > 650) {
    lastLCD = millis();
    lcd.clear();
    lcd.setCursor(0, 0);

    //print mode: dark, balanced, or dominant color
    if (redValue < 50 && blueValue < 50 && greenValue < 50) {
      lcd.print("DARK");
    } else {
      if (redValue   > greenValue && redValue   > blueValue) lcd.print("Color: Red");
      else if (greenValue > redValue   && greenValue > blueValue) lcd.print("Color: Green");
      else if (blueValue  > redValue   && blueValue  > greenValue) lcd.print("Color: Blue");
      else lcd.print("Balanced");
    }

    //frozen or live
    lcd.setCursor(0, 1);
    lcd.print(frozen ? "FROZEN" : "Live");

    //Serial monitor update
    Serial.print("R:"); Serial.print(redValue);
    Serial.print(" G:"); Serial.print(greenValue);
    Serial.print(" B:"); Serial.print(blueValue);
    Serial.println(frozen ? " [FROZEN]" : "");
  }
}