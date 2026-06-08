# 🌟 Tilt Stars

An interactive Arduino Uno project that simulates a night sky — tilt a sensor to light up LEDs and populate an OLED display with stars. Fill all 6 LEDs and trigger a shooting star animation across the screen.

---

##  Demo

> _Video demo coming soon._

---

##  Components

| Component | Details |
|---|---|
| Microcontroller | Arduino Uno |
| Display 1 | 0.96" I2C OLED |
| Display 2 | 16×2 I2C LCD |
| Input | Tilt sensor (ball switch) |
| Output | 6× LEDs |
| Other | Jumper wires, breadboard |

---

## How It Works

1. The LCD shows the current LED count and system status (`READY` / `WAIT`).
2. When status is **READY** (after a 6-second cooldown), tilt the sensor to trigger an event:
   - One LED lights up sequentially (pins 2 → 3 → 4 → 5 → 6 → 7).
   - 3 new stars appear on the OLED star field.
3. After all **6 LEDs** are lit, the next tilt:
   - Turns off all LEDs.
   - Clears and re-randomizes the star field.
   - Launches a **shooting star** animation diagonally across the OLED.
   - Resets the counter back to 0.
4. The cycle repeats indefinitely.

### Timing Constants

| Parameter | Value |
|---|---|
| Tilt cooldown | 6000 ms |
| Shooting star frame rate | 30 ms per frame |
| LCD blink period | 500 ms |

---

## OLED Display

- Header label: `* NIGHT SKY *`
- Stars rendered as 5-pixel shapes, randomly placed in the lower 3/4 of the screen.
- Stars flicker slightly each frame for a twinkling effect.
- Shooting star travels diagonally with a 3-pixel trailing tail.

---

## LCD Display

| Row | Content |
|---|---|
| Row 0 | `LEDs: X of 6` |
| Row 1 | Blinking `Tilt!` / `Status: READY` when ready; `Status: WAIT` / `Next in: Xs` during cooldown |

---

## 🔌 Wiring

### Tilt Sensor

| Sensor Pin | Arduino Pin |
|---|---|
| Signal | A3 |
| GND | GND |

_Configured as `INPUT_PULLUP` — triggers on LOW._

### LEDs

| LED # | Arduino Pin |
|---|---|
| 1 | D2 |
| 2 | D3 |
| 3 | D4 |
| 4 | D5 |
| 5 | D6 |
| 6 | D7 |

### OLED (SSD1306 — I2C)

| OLED Pin | Arduino Pin |
|---|---|
| VCC | 3.3V or 5V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

_I2C address: `0x3C`_

### LCD (LiquidCrystal I2C)

| LCD Pin | Arduino Pin |
|---|---|
| VCC | 5V |
| GND | GND |
| SDA | A4 |
| SCL | A5 |

_I2C address: `0x27` — both the OLED and LCD share the SDA/SCL bus (A4/A5)._

---

## 💡 Key Concepts Practiced

- **OLED animation** — frame-by-frame pixel management, shooting star, star field with twinkling effect
- **Tilt sensor input** — via time interval
- **Dual I2C display management** — OLED and LCD sharing the same bus

---
