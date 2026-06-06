# Smart Servo Motor Controller

An Arduino Uno project that combines multiple input methods — IR remote, joystick, and push button — to control a servo motor with real-time LCD feedback.

---
## Demo

 **[Watch the demo](https://drive.google.com/file/d/1eFq3uY_03Utb3-AOfIYQuruc04e991p4/view?usp=sharing)**

---

## Overview

The Smart Servo Motor Controller lets you position a servo motor using three different control methods simultaneously. An IR remote handles precise angle input and speed adjustment, a joystick provides smooth analog control, and a push button freezes/locks the servo at its current position. A 16x2 LCD keeps you informed of the current input and speed multiplier at all times.

---

## Components

| Component | Description |
|---|---|
| Arduino Uno | Main microcontroller board |
| Servo Motor | The primary output device being controlled |
| 16x2 LCD Display | Shows current input and speed multiplier |
| IR Remote + Receiver | Numeric input, directional control, speed adjustment |
| Joystick Module | Analog servo control with variable speed |
| Push Button | Lock/unlock toggle — freezes servo position |
| Jumper Wires | Connections between components |

---

## Wiring

| Component | Pin |
|---|---|
| LCD Display | 4, 5, 6, 7, 8, 9 |
| IR Receiver | 11 |
| Joystick (analog) | A4 |
| Push Button | 2 |
| Servo Motor | 3 |

---

## Features

### IR Remote Controls

| Button | Action |
|---|---|
| `▲` (Forward) | Servo → 0° |
| `▼` (Backward) | Servo → 180° |
| `+` | Increase speed multiplier (+0.5, max 3.5) |
| `-` | Decrease speed multiplier (−0.5, min 0.5) |
| `0–9` | Type a target angle digit by digit |
| `OK` | Confirm typed angle and move servo there |
| `TEST` | Move servo to a random angle (0–180°) |

### Joystick Control
- Analog input on A4 drives the servo smoothly
- A **deadzone** of ±50 around center prevents drift
- Movement speed is scaled by the current `speedMultiplier`

### Push Button
- **Press once** → locks the servo, displays current angle on LCD
- **Press again** → unlocks, clears the display, restores joystick control

### LCD Display
- **Line 1:** `Input: [typed digits]`
- **Line 2:** `Speed: [multiplier]`
- Updates only when values change (no unnecessary redraws)

---

## How Angle Input Works

1. Use the remote's number buttons to type an angle (e.g. `1`, `8`, `0` → "180")
2. Press **OK** to confirm — the servo moves to that angle, clamped to 0–180°
3. The input field clears automatically after confirmation

---

## What I Learned

- **Servo control** with the `Servo` library — attaching, writing angles, and constraining movement
- **IR remote decoding** with `IRremote` — reading raw 32-bit codes and mapping them to actions
- **Joystick reading** via `analogRead()` — centering, deadzone filtering, and scaled output
- **LCD optimization** — tracking last-displayed values to avoid unnecessary `lcd.print()` calls

---
