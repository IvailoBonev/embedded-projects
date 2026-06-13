#pragma once
#include <LovyanGFX.hpp>

static const int SCREEN_W  = 480;
static const int SCREEN_H  = 320;
static const int TOOLBAR_H = 50;
static const int CANVAS_Y  = TOOLBAR_H;
static const int CANVAS_H  = SCREEN_H - TOOLBAR_H;

static const int PAD          = 6;
static const int BTN_CLEAR_X  = PAD + 420;
static const int BTN_CLEAR_W  = 48;

static const int PEN_MAX = 20;

class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ILI9488  _panel;
  lgfx::Bus_SPI        _bus;
  lgfx::Touch_XPT2046  _touch;
public:
  LGFX();
};


extern LGFX lcd;

void displayInit();
void drawToolbar(bool forceAll = false);
void clearCanvas();
void renderDraw(int x, int y, int px, int py, int col, int sz);
void renderErase(int x, int y, int px, int py);
void renderShape(const char* shape, int x1, int y1, int x2, int y2, int col);
void renderText(int x, int y, const char* text, int col);
int hexToColor565(const char* hex);