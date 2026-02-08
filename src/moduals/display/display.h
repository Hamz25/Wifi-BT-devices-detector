#pragma once
#include <Adafruit_SSD1306.h>

// External display object
extern Adafruit_SSD1306 display;

// Display initialization
void display_init();

// Display modes
void display_radar();
void display_list(int selectedIndex);
void display_detail(int deviceIndex);

// Utility displays
void display_message(const char* message);
void display_connecting(const char* deviceName);