# Remote RGB LED Controller — ESP32

A Wi-Fi connected RGB LED controller built on the ESP32. Open a browser on any device on the same network, drag three sliders, hit a button — and the physical LED changes color in real time.

---

## Demo

[Watch the demo](https://drive.google.com/file/d/1pBwLNbFlPXtqYVz40LBJ2s5ODChjtJId/view?usp=sharing)

---

## Components

| Component | Purpose |
|---|---|
| ESP32 dev board | Microcontroller + Wi-Fi + PWM |
| RGB LED | Visual output |
| 3× resistors | Current limiting for each LED pin |
| Breadboard + jumper wires | Prototyping connections |

---

## Project Structure

```
├── src/
│   └── main.cpp          # ESP32 firmware (Arduino)
└── data/
    ├── index.html        # Web UI
    ├── script.js         # Slider logic + HTTP requests
    └── style.css         # Styling
```

> ⚠️ **SPIFFS Notice:** The `data/` folder must be uploaded to the ESP32's flash file system separately using the **"Upload Filesystem Image"** task in PlatformIO (or the SPIFFS upload tool for Arduino IDE). If you skip this step the ESP32 will return 404 for every page request.

---

## How It Works

### 1. Connecting to Wi-Fi

On startup the ESP32 reads the hardcoded `ssid` and `password` credentials and attempts to join the network. It blocks in a loop, printing dots to the serial monitor, until `WiFi.status() == WL_CONNECTED`. Once connected it prints the assigned IP address — that is the address you open in your browser.

```
Connecting......
Connected! → (your ip)
```

### 2. The Built-in Web Server

Rather than relying on any external server, the ESP32 itself runs an HTTP server on **port 80** — the standard web port — so no port number is needed in the URL. The `WebServer` object listens for incoming HTTP requests and calls different handler functions depending on the route (URL path) that was requested.

Three routes are registered:

| Route | Handler | What it does |
|---|---|---|
| `GET /` | `handleRoot()` | Serves `index.html` **or** sets the LED color |
| `GET /style.css` | `handleCSS()` | Serves the stylesheet |
| `GET /script.js` | `handleJS()` | Serves the JavaScript file |

### 3. Serving Files from SPIFFS

Instead of embedding the HTML/CSS/JS as raw strings inside the C++ code (which would be messy and hard to edit), the web files are stored in **SPIFFS** — the Serial Peripheral Interface Flash File System. This is a small file system that lives in a dedicated partition of the ESP32's onboard flash memory.

When a browser requests `/style.css`, for example, the handler opens `/style.css` from SPIFFS and streams it directly to the client:

```cpp
File file = SPIFFS.open(path, "r");
server.streamFile(file, contentType);
file.close();
```

This keeps the firmware clean and the web files independently editable.

### 4. Controlling the LED via PWM

A standard RGB LED has three separate internal LEDs (Red, Green, Blue) sharing a common ground. Each color channel is connected to its own GPIO pin through a current-limiting resistor, and each pin is driven by **PWM — Pulse Width Modulation**.

PWM rapidly switches a pin on and off. The ratio of on-time to off-time (the *duty cycle*) determines the perceived brightness. At 8-bit resolution, the duty cycle is a value from **0** to **255**. By mixing different duty cycles across the three channels, any color in the RGB space can be produced.

The ESP32 has a dedicated hardware PWM peripheral called **LEDC** (LED Control). Each channel must be initialized with a frequency and resolution before use:

```cpp
ledcSetup(CH_R, PWM_FREQ, PWM_RES);  // channel 0, 5kHz, 8-bit
ledcAttachPin(PIN_R, CH_R);          // attach GPIO 27 to channel 0
```

Once set up, writing a color is a single call per channel:

```cpp
ledcWrite(CH_R, r);  // r = 0–255
```

Three channels are used — one for Red (GPIO 27), Green (GPIO 26), and Blue (GPIO 25).

### 5. The Color Update Flow (End-to-End)

Here is the complete journey from slider movement to LED color change:

```
User drags slider in browser
        │
        ▼
script.js reads R, G, B values (0–255 each)
        │
        ▼
fetch("/?r=120&g=0&b=200")   ← HTTP GET with query parameters
        │
        ▼
ESP32 web server receives request on route "/"
        │
        ▼
handleRoot() detects ?r= ?g= ?b= arguments
        │
        ▼
setColor(r, g, b) writes duty cycles via ledcWrite()
        │
        ▼
LED physically changes color
        │
        ▼
Server responds with "200 OK"
```

The root handler does two things: if the request contains `r`, `g`, `b` query parameters it acts as a color command; if the request has no parameters it serves `index.html` as a normal page load.

### 6. The Web Interface

The UI is a single HTML page with three range sliders (one per channel). As any slider moves, a JavaScript function fires that:

- Updates the numeric label next to the slider
- Updates a live color preview box using `rgb(r, g, b)` inline CSS

When the user presses **Set Color**, a `fetch()` request fires to `/?r=…&g=…&b=…`, sending the current values to the ESP32. The **Reset Color** button sends `/?r=0&g=0&b=0`, turns off the LED, and resets the sliders back to zero.

---

## Wiring

The RGB LED has four legs. The longest leg is the common cathode (ground). Each of the other three legs connects to its respective GPIO through a resistor.

```
ESP32 GPIO 27  →  [Resistor]  →  LED Red pin
ESP32 GPIO 26  →  [Resistor]  →  LED Green pin
ESP32 GPIO 25  →  [Resistor]  →  LED Blue pin
ESP32 GND      →  LED Common Cathode
```

---

## Configuration

Remember to update your Wi-Fi credentials in `src/main.cpp`:

```cpp
const char* ssid     = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
```

---

## Quick Instructions

1. Open the project in PlatformIO.
2. Build and upload the firmware (`Upload` button or `pio run --target upload`).
3. Upload the file system image (`Upload Filesystem Image` task) — **this step is mandatory**.
4. Open the Serial Monitor at 115200 baud and wait for the IP address.
5. Open that IP in a browser on the same Wi-Fi network.

---

## Key Concepts Learned

**PWM on ESP32**: 
Unlike Arduinos where `analogWrite()` handles everything, the ESP32 requires explicit setup of PWM channels using `ledcSetup()`, `ledcAttachPin()`, and `ledcWrite()`. Multiple independent channels can run at different frequencies and resolutions simultaneously.

**Running a web server on a microcontroller**: 
The ESP32 can act as a full HTTP server on the local network. Routes are registered with callback functions, query parameters are parsed from the URL, and files can be streamed directly from flash storage.

**SPIFFS for separating firmware from web content**: 
Storing the UI files in the flash file system rather than hard-coding them as strings keeps the project organized. The file system image is uploaded as a separate step from the sketch itself.

---
