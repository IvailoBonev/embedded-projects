# 🎹 Digital Piano

An Arduino Uno-based digital piano that lets you play individual notes via a resistor ladder with buttons or play a full song automatically. Features real-time octave shifting displayed on an OLED screen, and note/frequency readout on an LCD screen.

---

## Demo

[Watch the demo](https://drive.google.com/file/d/16KUwOiQC4HlmN6CteH133VWXRfcGraCR/view?usp=sharing)

---

##  What It Does

- **Play notes** — Press one of four color-coded buttons (E, F, G, A) wired in a resistor ladder; the piezo buzzer plays the corresponding note
- **Change octave** — Tap the touch sensor to cycle through octaves 3–6; the OLED updates with the current octave number, navigation arrows, and indicator dots
- **Play a song** — Press the dedicated song button to auto-play a pre-programmed melody at 110 BPM with correct note durations
- **Live LCD** — The LCD displays the current note name + octave and its frequency in Hz while a button is held; shows `-- Ready --` when idle

---

## Components Used

| Component | Purpose |
|---|---|
| Arduino Uno | Main microcontroller |
| 16×2 LCD (I2C, 0x27) | Displays current note name and frequency |
| 0.96" OLED (I2C, 0x3C) | Displays current octave with arrows and dots |
| Piezo buzzer | Outputs tones for notes and song |
| Resistor ladder + 4 color buttons | Reads which note button is pressed via a single analog pin |
| Touch sensor | Cycles through octaves on tap |
| Normal button (song trigger) | Triggers automatic song playback |
| Resistors, jumper wires | Wiring and ladder voltage divider |

---

## Wiring

### Digital / Analog Pins

| Pin | Connection |
|---|---|
| `A2` | Resistor ladder output (note buttons) |
| `A3` | Song trigger button |
| `D2` | Touch sensor signal |
| `D8` | Piezo buzzer |

### I2C Bus (shared by LCD and OLED)

| Arduino Pin | Connection |
|---|---|
| `A4` (SDA) | LCD SDA + OLED SDA |
| `A5` (SCL) | LCD SCL + OLED SCL |
| `5V` | LCD VCC + OLED VCC |
| `GND` | LCD GND + OLED GND |

> **Note:** LCD I2C address is `0x27`; OLED I2C address is `0x3C`. Both share the same SDA/SCL lines.

### Resistor Ladder

The four note buttons are wired in a voltage-divider chain so each button produces a distinct analog value on `A2`:

| Button | Note | Approximate ADC Range |
|---|---|---|
| Button 1 | E4 | ≥ 950 |
| Button 2 | F4 | 800 – 920 |
| Button 3 | G4 | 400 – 600 |
| Button 4 | A4 | 80 – 110 |
| None | — | Outside all ranges |

> **Note:** You will probably have to fine tune your values if you try to replicate the project.
---

## How the Song Works

```cpp
int songNotes[]      = { 0, 1, 0, 1, 2, ... };   // index into E/F/G/A
float songDurations[] = { 0.5, 0.5, 1, 1, ... };  // 1 = quarter note, 0.5 = eighth note
```

Beat duration is calculated from BPM:

```cpp
float beatDuration = 60000.0 / bpm; // ms per beat
```

Each note duration is `songDurations[i] * beatDuration` ms, with a 10 ms gap between notes to prevent them from blurring together.

---

## Octave Shifting

```cpp
int getFreq(int index) {
  int shift = octave - 4;
  if (shift > 1) return baseNotes[index] << shift;
  if (shift > 0) return baseNotes[index] << shift;
  if (shift < 0) return baseNotes[index] >> (-shift);
  return baseNotes[index];
}
```

Octave range: **3 – 6**, cycling back to 3 after 6.

---

## What I Learned

- **Resistor ladder** — Reading multiple buttons through a single analog pin using a voltage divider chain
- **SSD1306 OLED over I2C** — Drawing text and shapes (filled triangles as arrows, circles as dots)
- **Dual I2C devices** — Running both an LCD and an OLED on the same SDA/SCL bus using different I2C addresses
- **`tone()` / `noTone()`** — Generating square-wave audio on a piezo buzzer with frequency control
- **BPM-based timing** — Converting musical note durations to milliseconds using a calculated beat duration

---