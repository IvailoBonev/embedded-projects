
//use of serial monitor, piezo and DHT11 sensor, potentiometer analog and millis() logic
#include <Arduino.h>
#include <DHT.h>

#define DHTPIN 7
#define BUZZER_PIN 6
#define POT_PIN A0
#define POTLED_PIN 9
#define BUTTON_PIN 2
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);
bool alarmEnabled = false; //the rapid beeping
bool lastButtonState = LOW;
int beepState = 0;
float tempC = 0;

unsigned long lastBeepTime = 0;
unsigned long lastDHTRead = 0;

void setup() {
    Serial.begin(9600);
    dht.begin();

    pinMode(BUTTON_PIN, INPUT);

    for (int i = 3; i < 6; i++) { pinMode(i, OUTPUT); digitalWrite(i, LOW); } //LEDs
    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(POTLED_PIN, OUTPUT);

    noTone(BUZZER_PIN);
}

void alarm(float t) {
    float ratio = (constrain(t, 25.0, 30.0) - 25.0) / 5.0;
    int pitch = 800 + ratio * 1300; // formual for pitch

    unsigned long beepDuration = 200 - ratio * 170; //how long
    unsigned long beepGap = 700 - ratio * 670; //how often

    //beep logic based on millis()
    if (beepState == 0 && millis() - lastBeepTime >= beepGap) {
        tone(BUZZER_PIN, pitch);
        lastBeepTime = millis();
        beepState = 1;
    } 
    else if (beepState == 1 && millis() - lastBeepTime >= beepDuration) {
        noTone(BUZZER_PIN);
        lastBeepTime = millis();
        beepState = 0;
    }
}

void loop() {
    // potentiometer
    int value = analogRead(POT_PIN);
    bool potActive = value > 30; //deadzone

    //LED and pitch control based on pot value
    if (potActive) {
        int brightness = constrain(map(value, 10, 1023, 0, 255), 0, 255);
        analogWrite(POTLED_PIN, brightness);

        int pitch = constrain(map(value, 10, 1023, 200, 3000), 200, 3000);
        tone(BUZZER_PIN, pitch);
    } else {
        analogWrite(POTLED_PIN, 0); // LED off when pot at 0
    } 

    // button toggle
    bool btn = digitalRead(BUTTON_PIN);
    if (btn == HIGH && lastButtonState == LOW) alarmEnabled = !alarmEnabled;
    lastButtonState = btn; 

    // read DHT every 2 seconds
    if (millis() - lastDHTRead >= 2000) {
        float newReading = dht.readTemperature();
        if (!isnan(newReading)) tempC = newReading; // only update if valid
        lastDHTRead = millis(); 
        //Serial monitor:
        Serial.print("Temp: ");
        Serial.print(tempC);
        Serial.println("°C");
    }

    //lit off LEDs based on temp
    digitalWrite(3, tempC >= 25.0 ? HIGH : LOW);
    digitalWrite(4, tempC >= 26.0 ? HIGH : LOW);
    digitalWrite(5, tempC >= 27.0 ? HIGH : LOW);    
    
    //when to alarm logic
    if (!potActive && tempC >= 25.0 && alarmEnabled) {
        alarm(tempC);
    } else if (!potActive) {
        noTone(BUZZER_PIN);
    }
}