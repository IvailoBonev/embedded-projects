#  Sound Bomb

An Arduino Uno project that combines a DHT11 temperature sensor, a piezo buzzer, a potentiometer, LEDs, and a push button to create a reactive sound and LEDs system. The buzzer alarm responds to temperature, while the potentiometer lets you manually control pitch and blue LED brightness.

---

##  Demo

[Watch the demo](https://drive.google.com/file/d/1r7DTlfSy9RRVYOmoM_5tx9GKGejtW-RR/view?usp=sharing)

---

## Features

- **Temperature Alarm Mode** — When temperature reaches or exceeds 25°C and the alarm is enabled, the buzzer beeps rhythmically. As temperature rises toward 30°C, the pitch increases and beeps become faster and shorter.
- **Potentiometer Mode** — Turning the potentiometer manually controls the buzzer pitch (200–3000 Hz) and dims/brightens a dedicated LED. This mode overrides the alarm.
- **LED Temperature Indicators** — Three LEDs light up progressively based on temperature thresholds (25°C, 26°C, 27°C).
- **Button Toggle** — A push button enables or disables the temperature alarm using `millis()`.
- **Serial Monitor Logging** — Temperature is printed to the Serial Monitor every 2 seconds.

---

## Components

| Component | Description |
|---|---|
| Arduino Uno | Main microcontroller board |
| DHT11 | Digital temperature & humidity sensor |
| Piezo Buzzer | Passive buzzer for tone output |
| Potentiometer | Analog input for pitch control |
| Push Button | Toggles the temperature alarm on/off |
| LEDs (x4) | 3× temperature indicators, 1× potentiometer brightness |
| Jumper Wires | For all connections |

---

## Wiring

| Component | Arduino Pin |
|---|---|
| DHT11 data | Digital **7** |
| Piezo Buzzer | Digital **6** |
| Potentiometer | Analog **A0** |
| Pot LED | Digital **9** (PWM) |
| Button | Digital **2** |
| LED 1 (≥25°C) | Digital **3** |
| LED 2 (≥26°C) | Digital **4** |
| LED 3 (≥27°C) | Digital **5** |

> **Note:** The button uses `INPUT` mode (not `INPUT_PULLUP`). Connect one leg to pin 2 and the other to 5V, with a 10kΩ pull-down resistor to GND.

---

##  Code Overview

### Key Logic

#### Alarm Function
```cpp
void alarm(float t) {
    float ratio = (constrain(t, 25.0, 30.0) - 25.0) / 5.0;
    int pitch = 800 + ratio * 1300;  // 800–2100 Hz
    unsigned long beepDuration = 200 - ratio * 170; // 30–200 ms
    unsigned long beepGap = 700 - ratio * 670; // 30–700 ms
}
```

As temperature climbs from 25°C to 30°C:
- Pitch rises from **800 Hz → 2100 Hz**
- Beep duration shortens from **200 ms → 30 ms**
- Gap between beeps shortens from **700 ms → 30 ms**

#### Mode Priority
```
Potentiometer active (value > 30)?
    YES → Manual pitch mode (overrides alarm)
    NO  → alarmEnabled AND tempC >= 25.0?
              YES → Temperature alarm mode
              NO  → Silence
```

#### Button Click
```cpp
bool btn = digitalRead(BUTTON_PIN);
if (btn == HIGH && lastButtonState == LOW)
    alarmEnabled = !alarmEnabled;
lastButtonState = btn;
```

---

## What I Learned

- **`millis()` non-blocking patterns** — Replacing `delay()`.
- **DHT11** — Reading temperature reliably by guarding against `NaN` values.
- **Piezo control** — Using `tone()` / `noTone()` to generate sound, and mapping analog input to pitch.
- **Analog input with a deadzone** — Ignoring low potentiometer values (< 30) to prevent false triggers from noisee.
- **Serial Monitor** — Using `Serial.print()` for real-time sensor feedback during development.

---