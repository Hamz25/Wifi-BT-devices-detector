#include "display.h"
#include "../tracking/tracking.h"
#include <Wire.h>
#include <math.h>
#include <algorithm>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Create display object
Adafruit_SSD1306 display(128, 64, &Wire, -1);

// Radar settings
#define RADAR_CENTER_X 64
#define RADAR_CENTER_Y 32
#define RADAR_MAX_RADIUS 30
#define MAX_DISPLAY_DISTANCE 20.0f  // meters
#define SCK_PIN 12
#define SDA_PIN 11
// Animation
static int radarAngle = 0;

void display_init() {
    // Force Pins for ESP32-S3
    Wire.begin(SCK_PIN, SDA_PIN); 

    // Start Hardware at address 0x3C
    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("Display Init Failed");
        return;
    }

    // Clear display
    display.clearDisplay();
    
    // Show startup message
    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(10, 20);
    display.println("DEVICE");
    display.setCursor(10, 40);
    display.println("TRACKER");
    display.display(); 
}

void display_radar() {
    display.clearDisplay();
    
    // Draw radar circles (range rings)
    for (int r = 10; r <= RADAR_MAX_RADIUS; r += 10) {
        display.drawCircle(RADAR_CENTER_X, RADAR_CENTER_Y, r, SSD1306_WHITE);
    }
    
    // Draw crosshair
    display.drawLine(RADAR_CENTER_X - RADAR_MAX_RADIUS, RADAR_CENTER_Y, 
                     RADAR_CENTER_X + RADAR_MAX_RADIUS, RADAR_CENTER_Y, SSD1306_WHITE);
    display.drawLine(RADAR_CENTER_X, RADAR_CENTER_Y - RADAR_MAX_RADIUS, 
                     RADAR_CENTER_X, RADAR_CENTER_Y + RADAR_MAX_RADIUS, SSD1306_WHITE);
    
    // Draw scanning line (rotating radar sweep)
    radarAngle = (radarAngle + 10) % 360;
    float angleRad = radarAngle * PI / 180.0;
    int x2 = RADAR_CENTER_X + RADAR_MAX_RADIUS * cos(angleRad);
    int y2 = RADAR_CENTER_Y + RADAR_MAX_RADIUS * sin(angleRad);
    display.drawLine(RADAR_CENTER_X, RADAR_CENTER_Y, x2, y2, SSD1306_WHITE);
    
    // Get all tracked devices
    std::vector<TrackedDevice> devices = tracking_getAllDevices();
    
    // Plot devices on radar
    for (const auto& dev : devices) {
        // Calculate position based on distance
        // Both are now doubles

        // Use (std::min) with parentheses to prevent macro expansion
        float normalizedDist = (std::min)((float)(dev.distance / MAX_DISPLAY_DISTANCE), 1.0f);
        int plotRadius = normalizedDist * RADAR_MAX_RADIUS;
        
        // Use lastSeen time to create a pseudo-angle distribution
        // This spreads devices around the circle instead of overlapping
        float deviceAngle = (dev.lastSeen % 360) * PI / 180.0;
        
        int x = RADAR_CENTER_X + plotRadius * cos(deviceAngle);
        int y = RADAR_CENTER_Y + plotRadius * sin(deviceAngle);
        
        // Draw device based on type
        if (dev.type == TYPE_WIFI_AP) {
            // WiFi AP = filled square
            display.fillRect(x - 2, y - 2, 4, 4, SSD1306_WHITE);
        } else if (dev.type == TYPE_BLUETOOTH) {
            // Bluetooth = circle
            display.drawCircle(x, y, 2, SSD1306_WHITE);
        } else {
            // Client = small dot
            display.drawPixel(x, y, SSD1306_WHITE);
        }
        
        // Mark new devices with a ring
        if (dev.isNew) {
            display.drawCircle(x, y, 4, SSD1306_WHITE);
        }
    }
    
    // Display device count at top
    display.setTextSize(0.5);
    display.setCursor(0, 0);
    display.printf("Devices: %d", devices.size());
    
    // Display legend at bottom
    display.setCursor(0, 56);
    display.print("[]=WiFi O=BLE");
    
    display.display();
}

void display_list(int selectedIndex) {
    display.clearDisplay();
    
    std::vector<TrackedDevice> devices = tracking_getAllDevices();
    
    // Title
    display.setTextSize(0.5);
    display.setCursor(0, 0);
    display.println("Device List");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Display up to 5 devices
    int startIdx = std::max(0, selectedIndex - 2);
    for (int i = 0; i < 5 && (startIdx + i) < devices.size(); i++) {
        int idx = startIdx + i;
        int y = 12 + i * 10;
        
        // Highlight selected
        if (idx == selectedIndex) {
            display.fillRect(0, y, 128, 10, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }
        
        display.setCursor(2, y + 1);
        
        const TrackedDevice& dev = devices[idx];
        String displayName = dev.name.isEmpty() ? 
            dev.mac.substring(0, 10) : dev.name.substring(0, 10);
        
        char typeChar = dev.type == TYPE_WIFI_AP ? 'W' : 
                        dev.type == TYPE_BLUETOOTH ? 'B' : 'C';
        
        display.printf("%c %s %.1fm", typeChar, displayName.c_str(), dev.distance);
    }
    
    display.display();
}

void display_detail(int deviceIndex) {
    display.clearDisplay();
    
    std::vector<TrackedDevice> devices = tracking_getAllDevices();
    
    if (deviceIndex >= devices.size()) {
        display.setCursor(0, 0);
        display.println("No device");
        display.display();
        return;
    }
    
    const TrackedDevice& dev = devices[deviceIndex];
    
    display.setTextSize(0.5);
    display.setTextColor(SSD1306_WHITE);
    
    // Device type
    display.setCursor(0, 0);
    if (dev.type == TYPE_WIFI_AP) {
        display.println("WiFi Access Point");
    } else if (dev.type == TYPE_BLUETOOTH) {
        display.println("Bluetooth Device");
    } else {
        display.println("WiFi Client");
    }
    
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Name/SSID
    display.setCursor(0, 14);
    display.print("Name: ");
    if (dev.name.isEmpty()) {
        display.println("Unknown");
    } else {
        display.println(dev.name.substring(0, 12));
    }
    
    // MAC Address
    display.setCursor(0, 24);
    display.print("MAC:");
    display.setCursor(0, 32);
    display.println(dev.mac.substring(0, 17));
    
    // Signal Strength
    display.setCursor(0, 42);
    display.printf("RSSI: %d dBm", dev.rssi);
    
    // Distance
    display.setCursor(0, 52);
    display.printf("Dist: %.2f m", dev.distance);
    
    display.display();
}

void display_message(const char* message) {
    display.clearDisplay();
    display.setTextSize(0.5);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 28);
    display.println(message);
    display.display();
}

void display_connecting(const char* deviceName) {
    display.clearDisplay();
    display.setTextSize(0.5);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Connecting to:");
    display.setCursor(0, 32);
    display.println(deviceName);
    display.display();
}