#pragma once
#include <Adafruit_SSD1306.h>

// External display object
extern Adafruit_SSD1306 display;

// Display initialization
void display_init();

// Display modes
void display_menu(const char** items, int itemCount, int selectedIndex, int scrollOffset);
void display_radar();
void display_list(int selectedIndex);
void display_detail(int deviceIndex, bool useMetric = true);
void display_packet_sniff(int channel, unsigned long totalPackets, int packetsPerSec);
void display_wifi_scan();
void display_bt_scan();
void display_stats();
void display_settings(const char** items, int itemCount, int selectedIndex, int scrollOffset,
                     unsigned long scanInterval, bool useMetric, bool autoScan, bool promiscuous);

// Utility displays
void display_message(const char* message);
void display_connecting(const char* deviceName);