#include "buttons.h"
#include "../display/display.h"
#include "../tracking/tracking.h"

// External variables from main.cpp
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

// External functions from wifi_scanner.h
extern void wifi_set_channel(uint8_t channel);
extern void wifi_enable_promiscuous();
extern void wifi_disable_promiscuous();

// Button instances - properly initialized (active HIGH)
Button btnUp    = {BTN_UP,    LOW, 0, 50};
Button btnDown  = {BTN_DOWN,  LOW, 0, 50};
Button btnLeft  = {BTN_LEFT,  LOW, 0, 50};
Button btnRight = {BTN_RIGHT, LOW, 0, 50};
// ============ BUTTON INITIALIZATION ============
void buttons_init() {
    // Initialize buttons with INTERNAL pull-down resistors (no external resistors needed!)
    // ESP32-S3 has built-in ~45kΩ pull-down resistors
    pinMode(BTN_UP, INPUT_PULLDOWN);
    pinMode(BTN_DOWN, INPUT_PULLDOWN);
    pinMode(BTN_LEFT, INPUT_PULLDOWN);
    pinMode(BTN_RIGHT, INPUT_PULLDOWN);
    
    Serial.println("Buttons initialized (active HIGH with INTERNAL pull-down resistors)");
    Serial.println("NO external resistors needed!");
    Serial.println("Button Controls:");
    Serial.println("  UP/DOWN: Navigate");
    Serial.println("  RIGHT: Select/Enter");
    Serial.println("  LEFT: Back/Exit");
}

// ============ BUTTON READING WITH DEBOUNCING ============
bool readButton(Button &btn) {
    int reading = digitalRead(btn.pin);
    bool pressed = false;
    
    if (reading != btn.lastState) {
        btn.lastDebounceTime = millis();
    }
    
    if ((millis() - btn.lastDebounceTime) > btn.debounceDelay) {
        // Button is active HIGH (pressed when reading is HIGH)
        if (reading == HIGH && btn.lastState == LOW) {
            pressed = true;
        }
    }
    
    btn.lastState = reading;
    return pressed;
}

// ============ MAIN BUTTON HANDLER ============
void handleButtons() {
    if (readButton(btnUp)) {
        handleUpButton();
    }
    if (readButton(btnDown)) {
        handleDownButton();
    }
    if (readButton(btnLeft)) {
        handleLeftButton();
    }
    if (readButton(btnRight)) {
        handleRightButton();
    }
}

// ============ UP BUTTON HANDLER ============
void handleUpButton() {
    switch (currentMode) {
        case MENU_MAIN:
            if (mainMenuIndex > 0) {
                mainMenuIndex--;
                if (mainMenuIndex < mainMenuScroll) {
                    mainMenuScroll = mainMenuIndex;
                }
            }
            break;
            
        case MODE_LIST:
        case MODE_DETAIL:
            if (selectedDevice > 0) {
                selectedDevice--;
                if (selectedDevice < deviceListScroll) {
                    deviceListScroll = selectedDevice;
                }
            }
            break;
            
        case MODE_SETTINGS:
            if (settingsIndex > 0) {
                settingsIndex--;
                if (settingsIndex < settingsScroll) {
                    settingsScroll = settingsIndex;
                }
            }
            break;
            
        case MODE_PACKET_SNIFF:
            // Change channel up
            if (currentSniffChannel < 14) {
                currentSniffChannel++;
                wifi_set_channel(currentSniffChannel);
            }
            break;
            
        default:
            break;
    }
}

// ============ DOWN BUTTON HANDLER ============
void handleDownButton() {
    // Main menu count (from main.cpp)
    const int mainMenuCount = 7;
    const int settingsCount = 5;
    
    switch (currentMode) {
        case MENU_MAIN:
            if (mainMenuIndex < mainMenuCount - 1) {
                mainMenuIndex++;
                if (mainMenuIndex >= mainMenuScroll + 5) {
                    mainMenuScroll = mainMenuIndex - 4;
                }
            }
            break;
            
        case MODE_LIST:
        case MODE_DETAIL: {
            int deviceCount = tracking_getDeviceCount();
            if (selectedDevice < deviceCount - 1) {
                selectedDevice++;
                if (selectedDevice >= deviceListScroll + 5) {
                    deviceListScroll = selectedDevice - 4;
                }
            }
            break;
        }
            
        case MODE_SETTINGS:
            if (settingsIndex < settingsCount - 1) {
                settingsIndex++;
                if (settingsIndex >= settingsScroll + 5) {
                    settingsScroll = settingsIndex - 4;
                }
            }
            break;
            
        case MODE_PACKET_SNIFF:
            // Change channel down
            if (currentSniffChannel > 1) {
                currentSniffChannel--;
                wifi_set_channel(currentSniffChannel);
            }
            break;
            
        default:
            break;
    }
}

// ============ LEFT BUTTON HANDLER (BACK) ============
void handleLeftButton() {
    switch (currentMode) {
        case MODE_RADAR:
        case MODE_LIST:
        case MODE_WIFI_SCAN:
        case MODE_BT_SCAN:
        case MODE_PACKET_SNIFF:
        case MODE_SETTINGS:
        case MODE_STATS:
            // Go back to main menu
            currentMode = MENU_MAIN;
            if (promiscuousMode && currentMode != MODE_PACKET_SNIFF) {
                wifi_disable_promiscuous();
                promiscuousMode = false;
            }
            break;
            
        case MODE_DETAIL:
            // Go back to list view
            currentMode = MODE_LIST;
            break;
            
        default:
            break;
    }
}

// ============ RIGHT BUTTON HANDLER (SELECT) ============
void handleRightButton() {
    switch (currentMode) {
        case MENU_MAIN:
            // Select menu item
            switch (mainMenuIndex) {
                case 0: currentMode = MODE_RADAR; break;
                case 1: 
                    currentMode = MODE_LIST; 
                    selectedDevice = 0; 
                    deviceListScroll = 0; 
                    break;
                case 2: currentMode = MODE_WIFI_SCAN; break;
                case 3: currentMode = MODE_BT_SCAN; break;
                case 4: 
                    currentMode = MODE_PACKET_SNIFF;
                    if (!promiscuousMode) {
                        wifi_enable_promiscuous();
                        promiscuousMode = true;
                        currentSniffChannel = 1;
                        wifi_set_channel(currentSniffChannel);
                    }
                    break;
                case 5: currentMode = MODE_STATS; break;
                case 6: 
                    currentMode = MODE_SETTINGS; 
                    settingsIndex = 0; 
                    settingsScroll = 0; 
                    break;
            }
            break;
            
        case MODE_LIST:
            // Go to device detail view
            if (tracking_getDeviceCount() > 0) {
                currentMode = MODE_DETAIL;
            }
            break;
            
        case MODE_SETTINGS:
            // Toggle/select setting
            switch (settingsIndex) {
                case 0: // Scan interval
                    SCAN_INTERVAL = (SCAN_INTERVAL == 1000) ? 3000 : 
                                   (SCAN_INTERVAL == 3000) ? 5000 : 
                                   (SCAN_INTERVAL == 5000) ? 10000 : 1000;
                    break;
                case 1: // Distance unit
                    useMetric = !useMetric;
                    break;
                case 2: // Auto scan
                    autoScan = !autoScan;
                    break;
                case 3: // Promiscuous mode
                    promiscuousMode = !promiscuousMode;
                    if (promiscuousMode) {
                        wifi_enable_promiscuous();
                    } else {
                        wifi_disable_promiscuous();
                    }
                    break;
                case 4: // Back
                    currentMode = MENU_MAIN;
                    break;
            }
            break;
            
        default:
            break;
    }
}