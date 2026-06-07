#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
//OLED, LCD I2C, touch sensor and resistor ladder with piezo and buttons

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
LiquidCrystal_I2C lcd(0x27, 16, 2);

//standard frequencies for E4, F4, G4, A4
int baseNotes[]     = {329, 349, 392, 440};
const char* names[] = {"E", "F", "G", "A"};

const int BUTTON_PIN = A2;
const int PIEZO_PIN  = 8;
const int TOUCH_PIN = 2;
const int SONG_PIN = A3;

// song data
int songNotes[] = {0, 1, 0, 1, 2, 2, 0, 1, 0, 1, 2, 2, 3, 2, 3, 2, 1, 1, 2, 1, 2, 1, 0, 0};
float songDurations[] = {0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1, 0.5, 0.5, 0.5, 0.5, 1, 1}; // 1 - quarter note; 0.5 - eighth note;
float bpm = 110;
float beatDuration = 60000.0 / bpm; // duration of one beat in ms

int octave   = 3;
int lastNote = -1;
int lastOctave = 3;
unsigned long lastTouch = 0;

// frequency calculation based on octave shift
int getFreq(int index) {
  int shift = octave - 4;
  if (shift > 1) return baseNotes[index] << shift;
  if (shift > 0) return baseNotes[index] << shift;
  if (shift < 0) return baseNotes[index] >> (-shift);
  return baseNotes[index];
}

// play the song
void playSong() {
  // calculate the length of the song based on the number of notes
  int songLength = sizeof(songNotes) / sizeof(songNotes[0]);

  for (int i = 0; i < songLength; i++) {
    int note = songNotes[i];
    float duration = songDurations[i] * beatDuration; // convert beat duration to ms

    //check for rest note (-1) 
    if (note == -1) {
      noTone(PIEZO_PIN);
    } else {
      tone(PIEZO_PIN, getFreq(note));
    }

    delay(duration);
    noTone(PIEZO_PIN); // always stop tone after each note
    delay(10);          // tiny gap between notes
  }
}

// oled display function to show octave shift
void drawOLED() {
  oled.clearDisplay();

  // top bar
  oled.fillRect(0, 0, 128, 12, SSD1306_WHITE);
  oled.setTextColor(SSD1306_BLACK);
  oled.setTextSize(1);
  oled.setCursor(30, 2);
  oled.print("OCTAVE SHIFT");

  // big octave number
  oled.setTextColor(SSD1306_WHITE);
  oled.setTextSize(4);
  oled.setCursor(52, 18);
  oled.print(octave);

  // octave dots at bottom (3 dots, current one filled)
  for (int i = 0; i < 4; i++) {
    int x = 32 + i * 20;
    if (i == octave - 3) oled.fillCircle(x, 57, 4, SSD1306_WHITE);
    else oled.drawCircle(x, 57, 4, SSD1306_WHITE);
  }

  // left arrow if not at min
  if (octave > 3) {
    oled.fillTriangle(4, 32, 14, 24, 14, 40, SSD1306_WHITE);
  }
  // right arrow if not at max
  if (octave < 6) {
    oled.fillTriangle(124, 32, 114, 24, 114, 40, SSD1306_WHITE);
  }

  oled.display();
}

// lcd display function to show current note and frequency
void showLCD(int index, int octave) {
  if (index == lastNote && octave == lastOctave) return; // no need to update if nothing changed
  lastNote = index;
  lastOctave = octave;

  lcd.clear();
  if (index == -1) { // no note being played
    lcd.setCursor(0, 0);
    lcd.print("--  Ready  --");
  } else {
    lcd.setCursor(0, 0);
    lcd.print("Note: ");
    lcd.print(names[index]);
    lcd.print(octave);

    lcd.setCursor(0, 1);
    lcd.print("Freq: ");
    lcd.print(getFreq(index));
    lcd.print(" Hz");
  }
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("--  Ready  --");
  pinMode(TOUCH_PIN, INPUT);
  pinMode(SONG_PIN, INPUT);

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  drawOLED();
  showLCD(-1, octave);
}

void loop() {
  // read the button value from the resistor ladder and print it for debugging
  int keyVal = analogRead(BUTTON_PIN);
  Serial.println(keyVal);

  // check for touch input to change octave
  if (digitalRead(TOUCH_PIN) == HIGH && millis() - lastTouch > 400)
  {
    lastTouch = millis();
    lastOctave = octave;
    if (octave == 6) {
      octave = 3;
    } else {
      octave = octave + 1;
    }

    showLCD(lastNote, octave);
    drawOLED();
  }

  // determine which note to play based on the keyVal
  if (keyVal >= 950) {
    tone(PIEZO_PIN, getFreq(0));
    showLCD(0, octave);
  }
  else if (keyVal >= 800 && keyVal <= 920) {
    tone(PIEZO_PIN, getFreq(1));
    showLCD(1, octave);
  }
  else if (keyVal >= 400 && keyVal <= 600) {
    tone(PIEZO_PIN, getFreq(2));
    showLCD(2, octave);
  }
  else if (keyVal >= 80 && keyVal <= 110) {
    tone(PIEZO_PIN, getFreq(3));
    showLCD(3, octave);
  }
  else { // no button pressed
    noTone(PIEZO_PIN);
    showLCD(-1, octave);
  }
  
  //check if play song button is pressed
  if (digitalRead(SONG_PIN) == HIGH)
  {
    playSong();
  }
  
}