#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "buttons/buttons.h"  // NEW: Include buttons module (includes MenuMode enum)
#include "display/display.h"
#include "wifi/wifi_scanner.h"
#include "bluetooth/bt_scanner.h"
#include "tracking/tracking.h"

// ESP32-S3 Specific Pins
#define SDA_PIN 11
#define SCK_PIN 12
#define LED_PIN 48

Adafruit_NeoPixel LED_RGB(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// ============ MENU SYSTEM ============
// Note: MenuMode enum is defined in buttons/buttons.h

MenuMode currentMode = MENU_MAIN;

// Main menu items
const char* mainMenuItems[] = {
    "Radar View",
    "Device List",
    "WiFi Scan",
    "BT Scan",
    "Packet Sniff",
    "Statistics",
    "Settings"
};
const int mainMenuCount = 7;
int mainMenuIndex = 0;
int mainMenuScroll = 0;

// Settings menu items
const char* settingsItems[] = {
    "Scan Interval",
    "Distance Unit",
    "Auto Scan",
    "Promiscuous",
    "Back"
};
const int settingsCount = 5;
int settingsIndex = 0;
int settingsScroll = 0;

// Display and scan variables
unsigned long lastScan = 0;
unsigned long SCAN_INTERVAL = 3000; // Adjustable scan interval (3 seconds default)
int selectedDevice = 0;
int deviceListScroll = 0;
bool autoScan = true;
bool promiscuousMode = false;

// Packet sniffing variables
unsigned long packetCount = 0;
unsigned long lastPacketTime = 0;
int packetsPerSecond = 0;
unsigned long ppsUpdateTime = 0;
int currentSniffChannel = 1;

// Distance settings
bool useMetric = true; // true = meters, false = feet

// ============ PACKET SNIFFING CALLBACK ============
// This is called from wifi.cpp - increment counter
void incrementPacketCount() {
    packetCount++;
    lastPacketTime = millis();
}

// ============ SETUP ============
void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println("\n=== ESP32-S3 Device Tracker ===");
    Serial.println("Enhanced with Navigation & Packet Sniffing");
    Serial.println("================================\n");
    
    // Initialize buttons (moved to buttons module)
    buttons_init();
    
    // Initialize RGB LED
    LED_RGB.begin();
    LED_RGB.setBrightness(50);
    LED_RGB.setPixelColor(0, LED_RGB.Color(255, 165, 0)); // Orange = Initializing
    LED_RGB.show();
    
    // Initialize I2C
    Wire.begin(SDA_PIN, SCK_PIN);
    
    // Initialize Display
    Serial.println("Initializing Display...");
    display_init();
    delay(2000); // Show startup message
    
    // Initialize WiFi Scanner
    Serial.println("Initializing WiFi...");
    wifi_init();
    
    // Initialize Bluetooth Scanner
    Serial.println("Initializing Bluetooth...");
    bt_init();
    
    // Initialize Tracker
    Serial.println("Initializing Tracker...");
    tracking_init();
    
    LED_RGB.setPixelColor(0, LED_RGB.Color(0, 255, 0)); // Green = Ready
    LED_RGB.show();
    
    Serial.println("\n=== System Ready ===\n");
}

// ============ MAIN LOOP ============
void loop() {
    unsigned long currentTime = millis();
    
    // Handle button inputs (moved to buttons module)
    handleButtons();
    
    // Perform periodic scans (if auto-scan enabled and not in packet sniff mode)
    if (autoScan && currentMode != MODE_PACKET_SNIFF && (currentTime - lastScan >= SCAN_INTERVAL)) {
        lastScan = currentTime;
        
        // LED: Scanning
        LED_RGB.setPixelColor(0, LED_RGB.Color(0, 0, 255)); // Blue = Scanning
        LED_RGB.show();
        
        Serial.println("--- Starting Scan ---");
        
        // Scan WiFi
        Serial.println("Scanning WiFi...");
        std::vector<Device> wifiDevices = wifi_scan();
        Serial.printf("Found %d WiFi networks\n", wifiDevices.size());
        
        // Scan Bluetooth
        Serial.println("Scanning Bluetooth...");
        std::vector<Device> btDevices = bt_scan();
        Serial.printf("Found %d Bluetooth devices\n", btDevices.size());
        
        // Update tracker with all devices
        tracking_update(wifiDevices, btDevices);
        
        // Print summary
        Serial.printf("\nTotal tracked devices: %d\n", tracking_getDeviceCount());
        Serial.println("--- Scan Complete ---\n");
        
        // LED: Ready
        LED_RGB.setPixelColor(0, LED_RGB.Color(0, 255, 0)); // Green = Ready
        LED_RGB.show();
    }
    
    // Manual scans for specific modes
    if (currentMode == MODE_WIFI_SCAN && (currentTime - lastScan >= 2000)) {
        lastScan = currentTime;
        std::vector<Device> wifiDevices = wifi_scan();
        std::vector<Device> empty;
        tracking_update(wifiDevices, empty);
    }
    else if (currentMode == MODE_BT_SCAN && (currentTime - lastScan >= 2000)) {
        lastScan = currentTime;
        std::vector<Device> btDevices = bt_scan();
        std::vector<Device> empty;
        tracking_update(empty, btDevices);
    }
    
    // Update packets per second calculation
    if (currentMode == MODE_PACKET_SNIFF && (currentTime - ppsUpdateTime >= 1000)) {
        packetsPerSecond = packetCount - (packetCount - packetsPerSecond);
        ppsUpdateTime = currentTime;
    }
    
    // Update display based on current mode
    switch (currentMode) {
        case MENU_MAIN:
            display_menu(mainMenuItems, mainMenuCount, mainMenuIndex, mainMenuScroll);
            break;
        case MODE_RADAR:
            display_radar();
            break;
        case MODE_LIST:
            display_list(selectedDevice);
            break;
        case MODE_DETAIL:
            display_detail(selectedDevice, useMetric);
            break;
        case MODE_WIFI_SCAN:
            display_wifi_scan();
            break;
        case MODE_BT_SCAN:
            display_bt_scan();
            break;
        case MODE_PACKET_SNIFF:
            display_packet_sniff(currentSniffChannel, packetCount, packetsPerSecond);
            break;
        case MODE_SETTINGS:
            display_settings(settingsItems, settingsCount, settingsIndex, settingsScroll, 
                           SCAN_INTERVAL, useMetric, autoScan, promiscuousMode);
            break;
        case MODE_STATS:
            display_stats();
            break;
    }
    
    delay(50); // Smooth UI updates
}