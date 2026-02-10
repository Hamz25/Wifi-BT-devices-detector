#pragma once
#include <Arduino.h>

// Button pin definitions - SAFE pins for ESP32-S3 (won't cause shutdown)
// These pins are safe for general GPIO use and won't interfere with boot
#define BTN_UP    14  // GPIO14 - Safe
#define BTN_DOWN  15  // GPIO15 - Safe  
#define BTN_LEFT  16  // GPIO16 - Safe
#define BTN_RIGHT 17  // GPIO17 - Safe

// Menu mode enumeration (shared with main.cpp)
enum MenuMode {
    MENU_MAIN,
    MODE_RADAR,
    MODE_LIST,
    MODE_DETAIL,
    MODE_WIFI_SCAN,
    MODE_BT_SCAN,
    MODE_PACKET_SNIFF,
    MODE_SETTINGS,
    MODE_STATS
};

// Button structure for debouncing
struct Button {
    int pin;
    bool lastState;
    unsigned long lastDebounceTime;
    unsigned long debounceDelay;  // Changed from const to allow initialization
};

// External button instances
extern Button btnUp;
extern Button btnDown;
extern Button btnLeft;
extern Button btnRight;

// Initialize all buttons
void buttons_init();

// Read button with debouncing - returns true on press
bool readButton(Button &btn);

// Handle all button inputs
void handleButtons();

// Individual button handlers
void handleUpButton();
void handleDownButton();
void handleLeftButton();
void handleRightButton();