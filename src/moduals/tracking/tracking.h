#pragma once
#include <vector>
#include <Arduino.h>
#include "../wifi/wifi_scanner.h"

// Extended device information with tracking data
struct TrackedDevice {
    String mac;
    String name;
    int rssi;
    float avgRSSI;  // Running average for stability
    float distance;
    DeviceType type;
    uint8_t channel;
    unsigned long firstSeen;
    unsigned long lastSeen;
    int seenCount;
    bool isNew;  // True if discovered in the last scan
};

// Initialize tracking system
void tracking_init();

// Update tracked devices with new scan results
void tracking_update(const std::vector<Device>& wifiDevices, 
                     const std::vector<Device>& btDevices);

// Get all tracked devices
std::vector<TrackedDevice> tracking_getAllDevices();

// Get devices filtered by type
std::vector<TrackedDevice> tracking_getDevicesByType(DeviceType type);

// Get devices within a certain distance
std::vector<TrackedDevice> tracking_getNearbyDevices(float maxDistance);

// Get a specific device by MAC address
TrackedDevice* tracking_getDeviceByMAC(const String& mac);

// Get device count
int tracking_getDeviceCount();

// Clear all tracked devices
void tracking_clear();

// Print tracking statistics
void tracking_printStats();