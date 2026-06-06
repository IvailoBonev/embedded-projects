# LED Timing Game

A simple reaction/timing game built on Arduino using 8 LEDs, two buttons, resistors and a breadboard. One of my earliest Arduino projects! **Note:** This code works even if you don't have physical pull up resistors for your buttons as I used **INPUT_PULLUP**. 

---

[Watch demo](https://drive.google.com/file/d/1oEnfp7SJMVzkz9ctN1HnkqSlsQEopM4Q/view)

---

## How It Works

The game cycles through 7 LEDs one at a time, lighting each one briefly before moving to the next — like a moving slot machine. Your goal is to press the main button at the right moment to stop the cursor on the yellow LED.

A second button acts as a **slow-motion modifier**: holding it down doubles the delay between each LED step, giving you more time to react. Release it and the cursor speeds back up.

---

## Gameplay

- **Press the trigger button** — the LEDs begin to move
- **Hold the slow button** — the LEDs slow down
- **Release the trigger button** — the cursor freezes on whichever LED it was at
- A separate green LED lights up while the game is active

The challenge is stopping the cursor exactly on the yellow LED — harder than it sounds at speed.

---

## Code Breakdown

```cpp
int pins[] = {4, 5, 6, 9, 10, 11, 12};
int currentLED = 0;
```

Seven output pins map to the seven LEDs. `currentLED` tracks which one is currently active.

---

```cpp
pinMode(2, INPUT_PULLUP);
pinMode(13, INPUT_PULLUP);
```

Both buttons use the Arduino's internal pull-up resistors. This means a pressed button reads `LOW`, not `HIGH` (inverted logic).

---

```cpp
if (switchState == LOW) {
  digitalWrite(3, HIGH);  // indicator LED on
  ...
  currentLED++;
  if (currentLED >= 7) currentLED = 0; //cycle again
  digitalWrite(pins[currentLED], HIGH);
```

When the trigger button is held, the indicator LED turns on and begins the loop logic for the 7 LEDs.

---

```cpp
if (digitalRead(13) == HIGH) {
  delay(45); 
} else {
  delay(100); 
}
```

The slow button (pin 13, also pull-up) controls the step delay. Released = 45ms per step (normal). Pressed = 100ms per step (slowed). You can swap these values to tune the difficulty.

---

```cpp
} else {
  digitalWrite(3, LOW);
  digitalWrite(pins[currentLED], HIGH);
}
```

When the trigger button is released, the indicator turns off and the loopfreezes — whatever LED was last active stays lit.

---

## Components Used

- Arduino Uno
- Breadboard
- LEDs (multiple colours — red, yellow, blue, green)
- Current-limiting resistors (one per LED)
- 10k Ohm resistors and 220 Ohm resistors
- Push buttons
- Jumper wires

---

## What I Learned

- Using `INPUT_PULLUP` to read button inputs
- Managing multiple output pins with an array
- The 'delay()' function and using multiple buttons at the same time

---

## Wiring Notes

| Pin | Function |
|-----|----------|
| 2   | Trigger button (INPUT_PULLUP) |
| 13  | Slow-mo button (INPUT_PULLUP) |
| 3   | Indicator LED |
| 4, 5, 6, 9, 10, 11, 12 | Game LEDs (left to right) |

Each LED pin connects through a current-limiting resistor to the LED anode, with cathode to GND.
