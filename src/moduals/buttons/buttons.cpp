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
    
    Serial.println("\n╔════════════════════════════════════════╗");
    Serial.println("║  BUTTONS INITIALIZED - DEBUG MODE ON  ║");
    Serial.println("╚════════════════════════════════════════╝");
    Serial.println("Config: INPUT_PULLDOWN (internal ~45kΩ)");
    Serial.println("Logic: Active HIGH (3.3V = pressed)");
    Serial.println();
    Serial.println("Pin Map:");
    Serial.printf("  GPIO %d → UP button\n", BTN_UP);
    Serial.printf("  GPIO %d → DOWN button\n", BTN_DOWN);
    Serial.printf("  GPIO %d → LEFT button\n", BTN_LEFT);
    Serial.printf("  GPIO %d → RIGHT button\n", BTN_RIGHT);
    Serial.println();
    Serial.println("⚠️  Wire: 3.3V → Button → GPIO pin");
    Serial.println("    NO external resistors needed!");
    Serial.println();
    
    // Test initial states
    Serial.println("Initial button readings:");
    Serial.printf("  UP=%d DOWN=%d LEFT=%d RIGHT=%d\n",
                  digitalRead(BTN_UP),
                  digitalRead(BTN_DOWN), 
                  digitalRead(BTN_LEFT),
                  digitalRead(BTN_RIGHT));
    Serial.println("  (Should all be 0 when not pressed, 1 when pressed)");
    Serial.println();
    Serial.println("Press buttons to see debug output...\n");
}

// ============ BUTTON READING WITH DEBOUNCING ============
bool readButton(Button &btn) {
    int reading = digitalRead(btn.pin);
    bool pressed = false;
    
    // If the reading is the same as the last KNOWN STABLE state, 
    // nothing has changed. Reset the timer so we don't trigger.
    if (reading == btn.lastState) {
        btn.lastDebounceTime = millis();
    }
    
    // If the reading is different (e.g., went from LOW to HIGH),
    // we wait for the debounceDelay to pass to be sure it's real.
    if ((millis() - btn.lastDebounceTime) > btn.debounceDelay) {
        
        // If we are here, the reading has been stable for >50ms.
        // Now we can update the state.
        
        if (reading != btn.lastState) {
            btn.lastState = reading; // Update the state NOW, not before.
            
            // If the new state is HIGH, that means it was just pressed.
            if (btn.lastState == HIGH) {
                pressed = true;
                Serial.printf("Button GPIO %d Pressed!\n", btn.pin);
            }
        }
    }
    
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
    Serial.printf("  UP: Mode=%d ", currentMode);
    
    switch (currentMode) {
        case MENU_MAIN:
            if (mainMenuIndex > 0) {
                mainMenuIndex--;
                if (mainMenuIndex < mainMenuScroll) {
                    mainMenuScroll = mainMenuIndex;
                }
                Serial.printf("Menu[%d]\n", mainMenuIndex);
            } else {
                Serial.println("(at top)");
            }
            break;
            
        case MODE_LIST:
        case MODE_DETAIL:
            if (selectedDevice > 0) {
                selectedDevice--;
                if (selectedDevice < deviceListScroll) {
                    deviceListScroll = selectedDevice;
                }
                Serial.printf("Device[%d]\n", selectedDevice);
            } else {
                Serial.println("(at top)");
            }
            break;
            
        case MODE_SETTINGS:
            if (settingsIndex > 0) {
                settingsIndex--;
                if (settingsIndex < settingsScroll) {
                    settingsScroll = settingsIndex;
                }
                Serial.printf("Setting[%d]\n", settingsIndex);
            } else {
                Serial.println("(at top)");
            }
            break;
            
        case MODE_PACKET_SNIFF:
            // Change channel up
            if (currentSniffChannel < 14) {
                currentSniffChannel++;
                wifi_set_channel(currentSniffChannel);
                Serial.printf("Ch=%d\n", currentSniffChannel);
            } else {
                Serial.println("(max ch)");
            }
            break;
            
        default:
            Serial.println("(no action)");
            break;
    }
}

// ============ DOWN BUTTON HANDLER ============
void handleDownButton() {
    Serial.printf("  DOWN: Mode=%d ", currentMode);
    
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
                Serial.printf("Menu[%d]\n", mainMenuIndex);
            } else {
                Serial.println("(at bottom)");
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
                Serial.printf("Device[%d]\n", selectedDevice);
            } else {
                Serial.println("(at bottom)");
            }
            break;
        }
            
        case MODE_SETTINGS:
            if (settingsIndex < settingsCount - 1) {
                settingsIndex++;
                if (settingsIndex >= settingsScroll + 5) {
                    settingsScroll = settingsIndex - 4;
                }
                Serial.printf("Setting[%d]\n", settingsIndex);
            } else {
                Serial.println("(at bottom)");
            }
            break;
            
        case MODE_PACKET_SNIFF:
            // Change channel down
            if (currentSniffChannel > 1) {
                currentSniffChannel--;
                wifi_set_channel(currentSniffChannel);
                Serial.printf("Ch=%d\n", currentSniffChannel);
            } else {
                Serial.println("(min ch)");
            }
            break;
            
        default:
            Serial.println("(no action)");
            break;
    }
}

// ============ LEFT BUTTON HANDLER (BACK) ============
void handleLeftButton() {
    Serial.printf("  LEFT: %d → ", currentMode);
    
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
            Serial.println("MENU_MAIN");
            if (promiscuousMode && currentMode != MODE_PACKET_SNIFF) {
                wifi_disable_promiscuous();
                promiscuousMode = false;
            }
            break;
            
        case MODE_DETAIL:
            // Go back to list view
            currentMode = MODE_LIST;
            Serial.println("MODE_LIST");
            break;
            
        default:
            Serial.println("(no action)");
            break;
    }
}

// ============ RIGHT BUTTON HANDLER (SELECT) ============
void handleRightButton() {
    Serial.printf("  Mode before: %d → ", currentMode);
    
    switch (currentMode) {
        case MENU_MAIN:
            // Select menu item
            Serial.printf("Menu[%d]: ", mainMenuIndex);
            switch (mainMenuIndex) {
                case 0: 
                    currentMode = MODE_RADAR; 
                    Serial.println("RADAR");
                    break;
                case 1: 
                    currentMode = MODE_LIST; 
                    selectedDevice = 0; 
                    deviceListScroll = 0; 
                    Serial.println("LIST");
                    break;
                case 2: 
                    currentMode = MODE_WIFI_SCAN;
                    Serial.println("WIFI_SCAN");
                    break;
                case 3: 
                    currentMode = MODE_BT_SCAN;
                    Serial.println("BT_SCAN");
                    break;
                case 4: 
                    currentMode = MODE_PACKET_SNIFF;
                    if (!promiscuousMode) {
                        wifi_enable_promiscuous();
                        promiscuousMode = true;
                        currentSniffChannel = 1;
                        wifi_set_channel(currentSniffChannel);
                    }
                    Serial.println("PACKET_SNIFF");
                    break;
                case 5: 
                    currentMode = MODE_STATS;
                    Serial.println("STATS");
                    break;
                case 6: 
                    currentMode = MODE_SETTINGS; 
                    settingsIndex = 0; 
                    settingsScroll = 0;
                    Serial.println("SETTINGS");
                    break;
            }
            break;
            
        case MODE_LIST:
            // Go to device detail view
            if (tracking_getDeviceCount() > 0) {
                currentMode = MODE_DETAIL;
                Serial.println("DETAIL");
            } else {
                Serial.println("(no devices)");
            }
            break;
            
        case MODE_SETTINGS:
            // Toggle/select setting
            switch (settingsIndex) {
                case 0: // Scan interval
                    SCAN_INTERVAL = (SCAN_INTERVAL == 1000) ? 3000 : 
                                   (SCAN_INTERVAL == 3000) ? 5000 : 
                                   (SCAN_INTERVAL == 5000) ? 10000 : 1000;
                    Serial.printf("Scan=%lums\n", SCAN_INTERVAL);
                    break;
                case 1: // Distance unit
                    useMetric = !useMetric;
                    Serial.printf("Unit=%s\n", useMetric ? "Metric" : "Imperial");
                    break;
                case 2: // Auto scan
                    autoScan = !autoScan;
                    Serial.printf("AutoScan=%s\n", autoScan ? "ON" : "OFF");
                    break;
                case 3: // Promiscuous mode
                    promiscuousMode = !promiscuousMode;
                    if (promiscuousMode) {
                        wifi_enable_promiscuous();
                    } else {
                        wifi_disable_promiscuous();
                    }
                    Serial.printf("Promiscuous=%s\n", promiscuousMode ? "ON" : "OFF");
                    break;
                case 4: // Back
                    currentMode = MENU_MAIN;
                    Serial.println("Back to MENU");
                    break;
            }
            break;
            
        default:
            Serial.println("(no action)");
            break;
    }
}