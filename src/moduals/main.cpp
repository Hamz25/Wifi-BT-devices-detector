#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_NeoPixel.h>
#include "display/display.h"
#include "wifi/wifi_scanner.h"
#include "bluetooth/bt_scanner.h"
#include "tracking/tracking.h"

// ESP32-S3 Specific Pins
#define SDA_PIN 11
#define SCK_PIN 12
#define LED_PIN 48

Adafruit_NeoPixel LED_RGB(1, LED_PIN, NEO_GRB + NEO_KHZ800);

// Display modes
enum DisplayMode {
    MODE_RADAR,
    MODE_LIST,
    MODE_DETAIL
};

DisplayMode currentMode = MODE_RADAR;
unsigned long lastScan = 0;
const unsigned long SCAN_INTERVAL = 1000; // Scan every 3 seconds
int selectedDevice = 0;

void setup() {
    Serial.begin(115200);
    while (!Serial) { delay(10); }
    
    Serial.println("\n=== ESP32-S3 Device Tracker ===");
    Serial.println("Educational Project - WiFi/BLE Scanner");
    Serial.println("================================\n");
    
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
    delay(2000); // Show "IT WORKS" message
    
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
    
    Serial.println("\n=== System Ready ===");
    Serial.println("Starting scans...\n");
}

void loop() {
    unsigned long currentTime = millis();
    
    // Perform periodic scans
    if (currentTime - lastScan >= SCAN_INTERVAL) {
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
        
        // Get all tracked devices
        std::vector<TrackedDevice> allDevices = tracking_getAllDevices();
        
        // Print summary
        Serial.printf("\nTotal tracked devices: %d\n", allDevices.size());
        for (const auto& dev : allDevices) {
            Serial.printf("  [%s] %s | %.1fm | %s\n", 
                dev.type == TYPE_WIFI_AP ? "WiFi" : 
                dev.type == TYPE_WIFI_CLIENT ? "Client" : "BLE",
                dev.name.isEmpty() ? dev.mac.c_str() : dev.name.c_str(),
                dev.distance,
                dev.isNew ? "NEW" : "Known");
        }
        Serial.println("--- Scan Complete ---\n");
        
        // LED: Ready
        LED_RGB.setPixelColor(0, LED_RGB.Color(0, 255, 0)); // Green = Ready
        LED_RGB.show();
    }
    
    // Update display based on current mode
    switch (currentMode) {
        case MODE_RADAR:
            display_radar();
            break;
        case MODE_LIST:
            display_list(selectedDevice);
            break;
        case MODE_DETAIL:
            display_detail(selectedDevice);
            break;
    }
    display.display();
    delay(100); // Smooth display updates
}