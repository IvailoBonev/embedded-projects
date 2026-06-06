//LED use and button uses
#include <Arduino.h>

int switchState = 0;
int pins[]  = {4, 5, 6, 9, 10, 11, 12};
int currentLED = 0;

void setup() {
  for (int i = 0; i < 7; i++) {
    pinMode(pins[i], OUTPUT);
  }

  pinMode(3, OUTPUT);

  pinMode(2, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
}

void loop() {
  switchState = digitalRead(2);

  if (switchState == LOW) {

  digitalWrite(3, HIGH);
    for (int i = 0; i < 7; i++) {
      digitalWrite(pins[i], LOW);
    }

    currentLED++;
    if (currentLED >= 7) {
      currentLED = 0;
    }

    digitalWrite(pins[currentLED], HIGH);

    if (digitalRead(13) == HIGH) {
      delay(45);
    }
    else {
      delay(100);
    }
  } 
  else {
    digitalWrite(3, LOW);
    digitalWrite(pins[currentLED], HIGH);
  }
}