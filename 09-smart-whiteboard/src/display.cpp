#include "display.h"
#include <Arduino.h>

//Global instances
LGFX lcd;

//LGFX constructor
LGFX::LGFX() {
  {
    auto cfg = _bus.config();
    cfg.spi_host    = VSPI_HOST;
    cfg.spi_mode    = 0;
    cfg.freq_write  = 40000000;
    cfg.freq_read   =  8000000;
    cfg.spi_3wire   = false;
    cfg.use_lock    = true;
    cfg.dma_channel = SPI_DMA_CH_AUTO;
    cfg.pin_sclk    = 13;
    cfg.pin_mosi    = 14;
    cfg.pin_miso    = -1;
    cfg.pin_dc      = 33;
    _bus.config(cfg);
    _panel.setBus(&_bus);
  }
  {
    auto cfg = _panel.config();
    cfg.pin_cs        = 26;
    cfg.pin_rst       = 25;
    cfg.pin_busy      = -1;
    cfg.memory_width  = 320;
    cfg.memory_height = 480;
    cfg.panel_width   = 320;
    cfg.panel_height  = 480;
    cfg.readable      = false;
    cfg.invert        = false;
    cfg.rgb_order     = false;
    cfg.dlen_16bit    = false;
    cfg.bus_shared    = false;
    _panel.config(cfg);
  }
  {
    auto cfg = _touch.config();
    cfg.pin_int    = 34;
    cfg.pin_sclk   = 32;
    cfg.pin_mosi   = 21;
    cfg.pin_miso   = 35;
    cfg.pin_cs     = 27;
    cfg.spi_host   = HSPI_HOST;
    cfg.bus_shared = false;
    cfg.offset_rotation = 0;
    _touch.config(cfg);
    _panel.setTouch(&_touch);
  }
  setPanel(&_panel);
}

void displayInit() {
  lcd.init();
  lcd.setRotation(1);

  lcd.fillScreen(TFT_WHITE);
  drawToolbar(true);
  clearCanvas();
}

//Toolbar
static void drawButton(int x, int w, int bg, int fg, const char* label) {
  lcd.fillRect(x, 1, w - 1, TOOLBAR_H - 2, bg);
  lcd.drawRect(x, 0, w, TOOLBAR_H, TFT_LIGHTGREY);
  lcd.setTextColor(fg);
  lcd.setTextSize(1);

  int tw = strlen(label) * 6;
  lcd.setCursor(x + (w - tw) / 2, TOOLBAR_H / 2 - 3);
  lcd.print(label);
}

void drawToolbar(bool forceAll) {
  if (forceAll)
    lcd.fillRect(0, 0, SCREEN_W, TOOLBAR_H, lcd.color565(50, 50, 50));
  if (forceAll)
    drawButton(BTN_CLEAR_X, BTN_CLEAR_W, lcd.color565(180,30,30), TFT_WHITE, "CLR");
}

//Canvas 
void clearCanvas() {
  lcd.fillRect(0, CANVAS_Y, SCREEN_W, CANVAS_H, TFT_WHITE);
}

static void clippedCircle(int cx, int cy, int r, int col) {
  for (int dy = -r; dy <= r; dy++) {
    int iy = cy + dy;

    if (iy < CANVAS_Y || iy >= SCREEN_H) continue;  
    
    int dx = (int)sqrtf((float)(r * r - dy * dy));
    int x1 = cx - dx;
    int x2 = cx + dx;

    if (x1 < 0)        x1 = 0;
    if (x2 >= SCREEN_W) x2 = SCREEN_W - 1;
    lcd.drawFastHLine(x1, iy, x2 - x1 + 1, col);
  }
}

//Render primitives
void renderDraw(int x, int y, int px, int py, int col, int sz) {
  int dy  = y  + CANVAS_Y;
  int dpy = py + CANVAS_Y;

  int steps = max(abs(x - px), abs(dy - dpy));

  if (steps == 0) steps = 1;
  for (int i = 0; i <= steps; i++) {
    int ix = px + (x - px) * i / steps;
    int iy = dpy + (dy - dpy) * i / steps;
    if (iy >= CANVAS_Y) clippedCircle(ix, iy, sz, col);
  }
}

void renderErase(int x, int y, int px, int py) {
  int dy  = y  + CANVAS_Y;
  int dpy = py + CANVAS_Y;

  int steps = max(abs(x - px), abs(dy - dpy));

  if (steps == 0) steps = 1;
  for (int i = 0; i <= steps; i++) {
    int ix = px + (x - px) * i / steps;
    int iy = dpy + (dy - dpy) * i / steps;
    if (iy >= CANVAS_Y) clippedCircle(ix, iy, PEN_MAX, TFT_WHITE);
  }
}

void renderShape(const char* shape, int x1, int y1, int x2, int y2, int col) {
  y1 += CANVAS_Y;
  y2 += CANVAS_Y;

  if (strcmp(shape, "rect") == 0) {
    lcd.drawRect(min(x1,x2), min(y1,y2), abs(x2-x1), abs(y2-y1), col);
  }
  else if (strcmp(shape, "circle") == 0) {
    lcd.drawEllipse(
      min(x1,x2) + abs(x2-x1)/2,
      min(y1,y2) + abs(y2-y1)/2,
      abs(x2-x1)/2, abs(y2-y1)/2, col);
  }
  else if (strcmp(shape, "line") == 0) {
    lcd.drawLine(x1, y1, x2, y2, col);
  }
}

void renderText(int x, int y, const char* text, int col) {
  lcd.setTextColor(col);
  lcd.setTextSize(2);
  lcd.setCursor(x, y + CANVAS_Y);
  lcd.print(text);
}

//Color helper
int hexToColor565(const char* hex) {
  if (!hex || hex[0] != '#') return TFT_WHITE;
  uint32_t raw = strtoul(hex + 1, nullptr, 16);
  uint8_t r = (raw >> 16) & 0xFF;
  uint8_t g = (raw >>  8) & 0xFF;
  uint8_t b =  raw        & 0xFF;
  return lcd.color565(r, g, b);
}