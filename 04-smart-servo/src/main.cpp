#include <Arduino.h>
#include <Servo.h>
#include <IRremote.h>
#include <LiquidCrystal.h>
//LCD, servo motor, remote, joystick

Servo servo;
LiquidCrystal lcd(4,5,6,7,8,9);

int IR_PIN = 11;
int JOY_PIN = A4;
int BUTTON_PIN = 2;
int SERVO_PIN = 3;

String inputDegrees = ""; //tracks input
String lastInput = ""; //what was last shown on the LCD
unsigned long lastClick;
float lastSpeed = -1;

unsigned long code = 0; //remote
bool locked = false;

float speedMultiplier = 1.0;

float minSpeed = 0.5;   // slowest
float maxSpeed = 3.5;  // fastest
int servoAngle = 90;    // starts centered

void setup() {
  servo.attach(SERVO_PIN);
  pinMode(BUTTON_PIN, INPUT);
  IrReceiver.begin(IR_PIN);
  
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Input:");
  lcd.setCursor(0, 1);
  lcd.print("Speed: 1.0");

  Serial.begin(9600);
}

void updateLCD() {
    // only update line 1 if input changed
    if (inputDegrees != lastInput) {
        lcd.setCursor(6, 0);  // position after "Input:"
        lcd.print("         ");  // clear
        lcd.setCursor(6, 0);
        lcd.print(inputDegrees);
        lastInput = inputDegrees;
    }

    // only update line 2 if speed changed
    if (speedMultiplier != lastSpeed) {
        lcd.setCursor(7, 1);  // position after "Speed: "
        lcd.print("         ");  // clear
        lcd.setCursor(7, 1);
        lcd.print(speedMultiplier);
        lastSpeed = speedMultiplier;
    }
}

void loop() {
    code = 0;

    //remote decoder
    if (IrReceiver.decode()) {
        code = IrReceiver.decodedIRData.decodedRawData;
        IrReceiver.resume();
    }

    //check if we got a valid code and we're not locked
    if (code != 0 && code != 0xFFFFFFFF && !locked) {
        if (code == 0xF609FF00) {           // forward
            servoAngle = 0;
            servo.write(servoAngle);
        }
        else if (code == 0xF807FF00) {      // backward
            servoAngle = 180;
            servo.write(servoAngle);
        }
        else if (code == 0xBB44FF00) {      // test
            servoAngle = random(0, 180);
            servo.write(servoAngle);
        }
        else if (code == 0xBF40FF00) {      // +
            speedMultiplier = constrain(speedMultiplier + 0.5, minSpeed, maxSpeed);
        }
        else if (code == 0xE619FF00) {      // -
            speedMultiplier = constrain(speedMultiplier - 0.5, minSpeed, maxSpeed);
        }
        else if (code == 0xE916FF00) { inputDegrees += "0"; }
        else if (code == 0xF30CFF00) { inputDegrees += "1"; }
        else if (code == 0xE718FF00) { inputDegrees += "2"; }
        else if (code == 0xA15EFF00) { inputDegrees += "3"; }
        else if (code == 0xF708FF00) { inputDegrees += "4"; }
        else if (code == 0xE31CFF00) { inputDegrees += "5"; }
        else if (code == 0xA55AFF00) { inputDegrees += "6"; }
        else if (code == 0xBD42FF00) { inputDegrees += "7"; }
        else if (code == 0xAD52FF00) { inputDegrees += "8"; }
        else if (code == 0xB54AFF00) { inputDegrees += "9"; }
        else if (code == 0xEA15FF00) {  // OK button
          if (inputDegrees.length() > 0) {  // only if something was typed
            int target = inputDegrees.toInt();
            target = constrain(target, 0, 180);
            servoAngle = target;
            servo.write(servoAngle);
            inputDegrees = "";  // reset for next entry
          }
        }
        updateLCD();
    }
    
    //lock toggle
    if (digitalRead(BUTTON_PIN) == HIGH && millis() - lastClick > 300) {
       lastClick = millis();
       locked = !locked;

       if (locked) {
        // just got locked — show locked angle on LCD
        inputDegrees = String(servoAngle);
        updateLCD();
      } 
      else {
          // just unlocked — clear the display
          inputDegrees = "";
          updateLCD();
      }
    }

    //joystick control (only if not locked)
    if (!locked) {
        int joyValue = analogRead(JOY_PIN);
        int centered = joyValue - 512;

        // deadzone - ignore tiny movements near center
        if (abs(centered) < 50) {
            centered = 0;
        }

        //--Serial output for debugging--
        //Serial.print("Joystick: ");
        //Serial.print(joyValue);
        //Serial.print(" | Speed: ");
        //Serial.print(speedMultiplier);
        //Serial.print(" | Angle: ");
        //Serial.println(servoAngle);

        servoAngle = servoAngle + (centered * speedMultiplier * 0.0055); //a bunch of tweaking here
        servoAngle = constrain(servoAngle, 0, 180);
        servo.write(servoAngle);
        delay(15);
    }
    
}