#  Color LED Lamp

An Arduino Uno project that uses three photoresistors to sense ambient light colors and drive an RGB LED accordingly — with a freeze mode, accent enhancement via potentiometer, and a live LCD status display.

---

##  Demo

[Watch Demo](https://drive.google.com/file/d/1lD41lem75B_RgMqo1gA52mOoTvMFXtgf/view?usp=sharing)

---

## Overview

The lamp continuously reads three photoresistors (one per color channel — R, G, B) and maps the light intensity to the corresponding RGB LED channel. A button lets you **freeze** the current color, at which point a **potentiometer accent** boosts the dominant color while suppressing the others. An LCD display shows the current mode and dominant color in real time.

---

## Features

| Feature | Description |
|---|---|
| **Live Mode** | RGB LED mirrors real-time photoresistor readings |
| **Freeze Mode** | Locks the current color; enables accent |
| **Accent** | Potentiometer boosts the dominant color and fades the rest |
| **Dark Mode** | Detects low-light conditions across all channels and enters a dark state |
| **Balanced Detection** | Recognizes when no single channel dominates |
| **LCD Display** | Shows dominant color (Red / Green / Blue / Balanced / DARK) and state |
| **Serial Monitor** | Streams R/G/B values and freeze state |

---

## Components

| Component | Notes |
|---|---|
| Arduino Uno | Main microcontroller |
| RGB LED (common cathode, 4-leg) | Red, Green, Blue channels |
| Photoresistors (LDR) | One per color channel |
| 16×2 LCD Display | LiquidCrystal, 4-bit mode |
| Potentiometer | Controls accent strength |
| Push Button | Toggles Live/Freeze mode |
| Resistors | Current limiting for LED legs and LDRs |
| Jumper Wires | - |

---

## Wiring

### RGB LED
| LED Pin | Arduino Pin |
|---|---|
| Red | D11 |
| Green | D10 |
| Blue | D9 |

### Photoresistors
| Sensor | Arduino Pin |
|---|---|
| Red channel LDR | A0 |
| Green channel LDR | A1 |
| Blue channel LDR | A2 |

### Controls & Display
| Component | Arduino Pin |
|---|---|
| Push Button | D12 |
| Potentiometer | A5 |
| LCD RS | D2 |
| LCD EN | D3 |
| LCD D4 | D4 |
| LCD D5 | D5 |
| LCD D6 | D6 |
| LCD D7 | D7 |

> Each photoresistor should be wired as a voltage divider with a pull-down resistor to GND.

---

## How It Works

### Live and Freeze Mode
```cpp
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
```
Sensor values are read from A0–A2, mapped from the 0–1023 ADC range to 0–255 PWM range, and written directly to the RGB LED pins.

### Freeze Activation (button press)
```cpp
if (digitalRead(buttonPin) == HIGH && millis() - lastButton > 600) {
    frozen     = !frozen;
    lastButton = millis();
  }
```
Sensor reading stops. The last captured R/G/B values are held. If one channel is clearly dominant, the **accent function** activates.

### Accent Function
```cpp
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
```
The accent is bypassed if the potentiometer is below a deadzone threshold (`< 30`) or if all channels read as dark (`< 50`).

### LCD Display Logic
```cpp
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
```

| Condition | Row 0 | Row 1 |
|---|---|---|
| All channels < 50 | `DARK` | `Live` or `FROZEN` |
| Red dominant | `Color: Red` | `Live` or `FROZEN` |
| Green dominant | `Color: Green` | `Live` or `FROZEN` |
| Blue dominant | `Color: Blue` | `Live` or `FROZEN` |
| No dominant channel | `Balanced` | `Live` or `FROZEN` |

---

## What I Learned

- **RGB LED management** via (`analogWrite`)
- **16×2 LCD** initialization and updating using `LiquidCrystal`
- **Photoresistors** to control the RGB LED
- **Non-blocking LCD refresh** using `millis()` intervals instead of `delay()`

---
