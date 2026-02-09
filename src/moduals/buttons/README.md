# Buttons Module

This module handles all button input for the WiFi/BT Device Tracker project.

## 📁 Files

- **buttons.h** - Header file with button definitions and function declarations
- **buttons.cpp** - Implementation file with button handling logic

## 🎮 Button Configuration

### Pin Definitions (Edit in buttons.h)
```cpp
#define BTN_UP    14   // Change to your actual UP button pin
#define BTN_DOWN  15   // Change to your actual DOWN button pin
#define BTN_LEFT  16   // Change to your actual LEFT button pin
#define BTN_RIGHT 17   // Change to your actual RIGHT button pin
```

### Hardware Setup
- All buttons use **INPUT_PULLUP** (internal pull-up resistors)
- Buttons are **active LOW** (press = connect to ground)
- **50ms debouncing** built-in to prevent double-presses

### Wiring Diagram
```
Button -> GPIO Pin -> ESP32-S3
UP     -> Pin 14
DOWN   -> Pin 15
LEFT   -> Pin 16
RIGHT  -> Pin 17

Each button:
[Button] --- [GPIO Pin]
    |
   GND
```

## 🔧 Functions

### Public Functions (buttons.h)

#### `void buttons_init()`
Initializes all buttons with pull-up resistors and prints control information.
```cpp
void setup() {
    buttons_init();  // Call this in setup()
}
```

#### `bool readButton(Button &btn)`
Reads a button with debouncing. Returns `true` when button is pressed.
```cpp
if (readButton(btnUp)) {
    // Button was pressed
}
```

#### `void handleButtons()`
Main button handler - checks all buttons and calls appropriate handlers.
```cpp
void loop() {
    handleButtons();  // Call this in main loop
}
```

#### Individual Button Handlers
- `void handleUpButton()` - UP button pressed
- `void handleDownButton()` - DOWN button pressed
- `void handleLeftButton()` - LEFT button pressed (back)
- `void handleRightButton()` - RIGHT button pressed (select)

## 🎯 Button Behavior by Mode

### MAIN MENU
- **UP/DOWN**: Navigate menu items
- **RIGHT**: Select current item
- **LEFT**: (disabled in main menu)

### RADAR VIEW
- **LEFT**: Back to main menu
- **UP/DOWN/RIGHT**: (no action)

### DEVICE LIST
- **UP/DOWN**: Scroll through devices
- **RIGHT**: View device details
- **LEFT**: Back to main menu

### DEVICE DETAIL
- **UP/DOWN**: Switch to next/previous device
- **LEFT**: Back to device list
- **RIGHT**: (no action)

### WIFI SCAN MODE
- **LEFT**: Back to main menu
- **UP/DOWN/RIGHT**: (no action, auto-refreshing)

### BLUETOOTH SCAN MODE
- **LEFT**: Back to main menu
- **UP/DOWN/RIGHT**: (no action, auto-refreshing)

### PACKET SNIFF MODE
- **UP**: Increase WiFi channel (1→14)
- **DOWN**: Decrease WiFi channel (14→1)
- **LEFT**: Back to main menu (disables promiscuous mode)
- **RIGHT**: (no action)

### SETTINGS MENU
- **UP/DOWN**: Navigate settings
- **RIGHT**: Toggle current setting
- **LEFT**: Back to main menu

### STATISTICS VIEW
- **LEFT**: Back to main menu
- **UP/DOWN/RIGHT**: (no action)

## 🔗 External Dependencies

The buttons module requires access to these external variables from `main.cpp`:

### Enums
```cpp
enum MenuMode {
    MENU_MAIN, MODE_RADAR, MODE_LIST, MODE_DETAIL,
    MODE_WIFI_SCAN, MODE_BT_SCAN, MODE_PACKET_SNIFF,
    MODE_SETTINGS, MODE_STATS
};
```

### Variables (must be declared in main.cpp)
```cpp
extern MenuMode currentMode;
extern int mainMenuIndex;
extern int mainMenuScroll;
extern int selectedDevice;
extern int deviceListScroll;
extern int settingsIndex;
extern int settingsScroll;
extern int currentSniffChannel;
extern unsigned long SCAN_INTERVAL;
extern bool useMetric;
extern bool autoScan;
extern bool promiscuousMode;
```

### Functions (must be implemented elsewhere)
```cpp
extern void wifi_set_channel(uint8_t channel);      // From wifi_scanner.h
extern void wifi_enable_promiscuous();              // From wifi_scanner.h
extern void wifi_disable_promiscuous();             // From wifi_scanner.h
extern int tracking_getDeviceCount();               // From tracking.h
```

## 📝 Usage Example

### In main.cpp:
```cpp
#include "buttons/buttons.h"

// Declare required external variables
MenuMode currentMode = MENU_MAIN;
int mainMenuIndex = 0;
int mainMenuScroll = 0;
// ... etc

void setup() {
    Serial.begin(115200);
    
    // Initialize buttons
    buttons_init();
    
    // ... rest of setup
}

void loop() {
    // Handle button inputs
    handleButtons();
    
    // ... rest of loop
}
```

## 🐛 Troubleshooting

### Buttons not responding?
1. Check pin numbers in `buttons.h` match your hardware
2. Verify buttons are wired to ground (active LOW)
3. Ensure `buttons_init()` is called in `setup()`
4. Check `handleButtons()` is called in `loop()`

### Double-presses occurring?
- Debounce delay is 50ms by default
- Increase `debounceDelay` in button struct if needed:
```cpp
struct Button {
    // ...
    const unsigned long debounceDelay = 100; // Increase to 100ms
};
```

### Buttons triggering wrong actions?
- Verify all external variables are properly declared in main.cpp
- Check that MenuMode enum matches between files
- Ensure wifi and tracking functions are available

## 🔄 Customization

### Adding New Button Actions

1. Add action to appropriate handler in `buttons.cpp`:
```cpp
void handleUpButton() {
    switch (currentMode) {
        case YOUR_NEW_MODE:
            // Your custom action
            break;
    }
}
```

2. Make sure new mode is in MenuMode enum in main.cpp

### Changing Button Pins

Edit the `#define` statements in `buttons.h`:
```cpp
#define BTN_UP    YOUR_PIN_NUMBER
```

### Disabling Debouncing

Set debounceDelay to 0 (not recommended):
```cpp
struct Button {
    const unsigned long debounceDelay = 0;
};
```

## 📊 Memory Usage

- **RAM**: ~20 bytes (4 Button structs)
- **Flash**: ~2-3 KB (button handling code)
- **No dynamic allocation**

## ✅ Feature Checklist

- [x] Debounced button reading
- [x] Support for 4 buttons (UP/DOWN/LEFT/RIGHT)
- [x] Mode-specific behavior
- [x] Scrollable menu navigation
- [x] Device list scrolling
- [x] Channel hopping in packet sniff
- [x] Settings toggling
- [x] Back navigation
- [x] Pull-up resistor support
- [x] Easy pin reconfiguration