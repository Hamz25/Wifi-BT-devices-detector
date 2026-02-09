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

// Display constants for 0.96" screen
#define MAX_VISIBLE_LINES 5
#define LINE_HEIGHT 11
#define MENU_START_Y 12

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

// ============ MAIN MENU DISPLAY ============
void display_menu(const char** items, int itemCount, int selectedIndex, int scrollOffset) {
    display.clearDisplay();
    
    // Title bar
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("=== MAIN MENU ===");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Display menu items (up to 5 visible)
    for (int i = 0; i < MAX_VISIBLE_LINES && (scrollOffset + i) < itemCount; i++) {
        int idx = scrollOffset + i;
        int y = MENU_START_Y + i * LINE_HEIGHT;
        
        // Highlight selected item
        if (idx == selectedIndex) {
            display.fillRect(0, y, 128, LINE_HEIGHT, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }
        
        display.setCursor(4, y + 2);
        display.print(items[idx]);
    }
    
    // Scroll indicators
    if (scrollOffset > 0) {
        // Up arrow
        display.fillTriangle(124, MENU_START_Y, 120, MENU_START_Y + 3, 128, MENU_START_Y + 3, SSD1306_WHITE);
    }
    if (scrollOffset + MAX_VISIBLE_LINES < itemCount) {
        // Down arrow
        display.fillTriangle(120, 60, 128, 60, 124, 57, SSD1306_WHITE);
    }
    
    display.display();
}

// ============ RADAR DISPLAY ============
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
        float normalizedDist = (std::min)((float)(dev.distance / MAX_DISPLAY_DISTANCE), 1.0f);
        int plotRadius = normalizedDist * RADAR_MAX_RADIUS;
        
        // Use lastSeen time to create a pseudo-angle distribution
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
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("Dev:%d", devices.size());
    
    // Display legend at bottom
    display.setCursor(0, 56);
    display.print("[]=WiFi O=BLE");
    
    display.display();
}

// ============ DEVICE LIST DISPLAY ============
void display_list(int selectedIndex) {
    display.clearDisplay();
    
    std::vector<TrackedDevice> devices = tracking_getAllDevices();
    
    // Title
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("Devices (%d)", devices.size());
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    if (devices.size() == 0) {
        display.setCursor(20, 28);
        display.print("No devices");
        display.display();
        return;
    }
    
    // Calculate scroll offset
    int scrollOffset = std::max(0, selectedIndex - 2);
    
    // Display up to 5 devices
    for (int i = 0; i < MAX_VISIBLE_LINES && (scrollOffset + i) < devices.size(); i++) {
        int idx = scrollOffset + i;
        int y = MENU_START_Y + i * 10;
        
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
            dev.mac.substring(0, 8) : dev.name.substring(0, 8);
        
        char typeChar = dev.type == TYPE_WIFI_AP ? 'W' : 
                        dev.type == TYPE_BLUETOOTH ? 'B' : 'C';
        
        display.printf("%c %s %.1fm", typeChar, displayName.c_str(), dev.distance);
    }
    
    // Scroll indicators
    if (scrollOffset > 0) {
        display.fillTriangle(124, MENU_START_Y, 120, MENU_START_Y + 3, 128, MENU_START_Y + 3, SSD1306_WHITE);
    }
    if (scrollOffset + MAX_VISIBLE_LINES < devices.size()) {
        display.fillTriangle(120, 60, 128, 60, 124, 57, SSD1306_WHITE);
    }
    
    display.display();
}

// ============ DEVICE DETAIL DISPLAY ============
void display_detail(int deviceIndex, bool useMetric) {
    display.clearDisplay();
    
    std::vector<TrackedDevice> devices = tracking_getAllDevices();
    
    if (deviceIndex >= devices.size()) {
        display.setCursor(0, 0);
        display.println("No device");
        display.display();
        return;
    }
    
    const TrackedDevice& dev = devices[deviceIndex];
    
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    
    // Device type (shortened for small screen)
    display.setCursor(0, 0);
    if (dev.type == TYPE_WIFI_AP) {
        display.println("WiFi AP");
    } else if (dev.type == TYPE_BLUETOOTH) {
        display.println("Bluetooth");
    } else {
        display.println("WiFi Client");
    }
    
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Name/SSID (scrollable if too long)
    display.setCursor(0, 12);
    display.print("N:");
    if (dev.name.isEmpty()) {
        display.println("Unknown");
    } else {
        // Truncate to fit screen
        String truncName = dev.name.length() > 19 ? dev.name.substring(0, 19) : dev.name;
        display.println(truncName);
    }
    
    // MAC Address (two lines for readability)
    display.setCursor(0, 21);
    display.print("M:");
    display.setCursor(0, 30);
    display.println(dev.mac);
    
    // Signal Strength
    display.setCursor(0, 39);
    display.printf("RSSI:%ddBm", dev.rssi);
    
    // Distance (with unit conversion)
    display.setCursor(0, 48);
    if (useMetric) {
        display.printf("Dist:%.2fm", dev.distance);
    } else {
        float distFeet = dev.distance * 3.28084;
        display.printf("Dist:%.2fft", distFeet);
    }
    
    // Additional info
    display.setCursor(0, 57);
    display.printf("Seen:%d", dev.seenCount);
    
    display.display();
}

// ============ PACKET SNIFFING DISPLAY ============
void display_packet_sniff(int channel, unsigned long totalPackets, int packetsPerSec) {
    display.clearDisplay();
    
    // Title
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("PACKET SNIFFER");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Current channel
    display.setCursor(0, 14);
    display.printf("Channel: %d", channel);
    
    // Packet count
    display.setCursor(0, 26);
    display.printf("Total: %lu", totalPackets);
    
    // Packets per second
    display.setCursor(0, 38);
    display.printf("Rate: %d p/s", packetsPerSec);
    
    // Visual activity bar
    int barWidth = (packetsPerSec > 100) ? 100 : packetsPerSec;
    display.drawRect(0, 50, 128, 10, SSD1306_WHITE);
    if (barWidth > 0) {
        display.fillRect(2, 52, (barWidth * 124) / 100, 6, SSD1306_WHITE);
    }
    
    // Instructions
    display.setTextSize(1);
    display.setCursor(0, 62);
    display.print("UP/DN:CH");
    
    display.display();
}

// ============ WIFI SCAN DISPLAY ============
void display_wifi_scan() {
    display.clearDisplay();
    
    std::vector<TrackedDevice> devices = tracking_getDevicesByType(TYPE_WIFI_AP);
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("WiFi APs (%d)", devices.size());
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    if (devices.size() == 0) {
        display.setCursor(10, 28);
        display.print("Scanning...");
        display.display();
        return;
    }
    
    // Show top 5 closest WiFi APs
    std::sort(devices.begin(), devices.end(),
        [](const TrackedDevice& a, const TrackedDevice& b) {
            return a.distance < b.distance;
        });
    
    for (int i = 0; i < MAX_VISIBLE_LINES && i < devices.size(); i++) {
        int y = MENU_START_Y + i * 10;
        display.setCursor(2, y);
        
        String ssid = devices[i].name.isEmpty() ? "Hidden" : devices[i].name.substring(0, 10);
        display.printf("%s %ddBm", ssid.c_str(), devices[i].rssi);
    }
    
    display.display();
}

// ============ BLUETOOTH SCAN DISPLAY ============
void display_bt_scan() {
    display.clearDisplay();
    
    std::vector<TrackedDevice> devices = tracking_getDevicesByType(TYPE_BLUETOOTH);
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.printf("BLE Dev (%d)", devices.size());
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    if (devices.size() == 0) {
        display.setCursor(10, 28);
        display.print("Scanning...");
        display.display();
        return;
    }
    
    // Show top 5 closest BLE devices
    std::sort(devices.begin(), devices.end(),
        [](const TrackedDevice& a, const TrackedDevice& b) {
            return a.distance < b.distance;
        });
    
    for (int i = 0; i < MAX_VISIBLE_LINES && i < devices.size(); i++) {
        int y = MENU_START_Y + i * 10;
        display.setCursor(2, y);
        
        String name = devices[i].name.substring(0, 10);
        display.printf("%s %ddBm", name.c_str(), devices[i].rssi);
    }
    
    display.display();
}

// ============ STATISTICS DISPLAY ============
void display_stats() {
    display.clearDisplay();
    
    std::vector<TrackedDevice> all = tracking_getAllDevices();
    int wifiCount = 0, bleCount = 0, clientCount = 0;
    
    for (const auto& dev : all) {
        if (dev.type == TYPE_WIFI_AP) wifiCount++;
        else if (dev.type == TYPE_BLUETOOTH) bleCount++;
        else clientCount++;
    }
    
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("STATISTICS");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    display.setCursor(0, 14);
    display.printf("Total: %d", all.size());
    
    display.setCursor(0, 26);
    display.printf("WiFi APs: %d", wifiCount);
    
    display.setCursor(0, 38);
    display.printf("BLE: %d", bleCount);
    
    display.setCursor(0, 50);
    display.printf("Clients: %d", clientCount);
    
    display.display();
}

// ============ SETTINGS DISPLAY ============
void display_settings(const char** items, int itemCount, int selectedIndex, int scrollOffset,
                     unsigned long scanInterval, bool useMetric, bool autoScan, bool promiscuous) {
    display.clearDisplay();
    
    // Title
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SETTINGS");
    display.drawLine(0, 10, 128, 10, SSD1306_WHITE);
    
    // Display settings (up to 5 visible)
    for (int i = 0; i < MAX_VISIBLE_LINES && (scrollOffset + i) < itemCount; i++) {
        int idx = scrollOffset + i;
        int y = MENU_START_Y + i * 10;
        
        // Highlight selected
        if (idx == selectedIndex) {
            display.fillRect(0, y, 128, 10, SSD1306_WHITE);
            display.setTextColor(SSD1306_BLACK);
        } else {
            display.setTextColor(SSD1306_WHITE);
        }
        
        display.setCursor(2, y + 1);
        
        // Display setting with current value
        switch (idx) {
            case 0: // Scan interval
                display.printf("Scan:%lus", scanInterval / 1000);
                break;
            case 1: // Distance unit
                display.printf("Unit:%s", useMetric ? "m" : "ft");
                break;
            case 2: // Auto scan
                display.printf("Auto:%s", autoScan ? "ON" : "OFF");
                break;
            case 3: // Promiscuous
                display.printf("Prom:%s", promiscuous ? "ON" : "OFF");
                break;
            case 4: // Back
                display.print("Back");
                break;
        }
    }
    
    display.display();
}

// ============ UTILITY DISPLAYS ============
void display_message(const char* message) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 28);
    display.println(message);
    display.display();
}

void display_connecting(const char* deviceName) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 20);
    display.println("Connecting to:");
    display.setCursor(0, 32);
    display.println(deviceName);
    display.display();
}