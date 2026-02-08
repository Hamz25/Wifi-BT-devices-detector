#include "tracking.h"
#include <algorithm>

// Storage for tracked devices
static std::vector<TrackedDevice> trackedDevices;
static unsigned long lastUpdateTime = 0;

// Timeout for removing inactive devices (milliseconds)
const unsigned long DEVICE_TIMEOUT = 60000; // 1 minute

void tracking_init() {
    trackedDevices.clear();
    Serial.println("Device tracking initialized");
}

// Find a device in the tracked list by MAC address
int findDeviceIndex(const String& mac) {
    for (size_t i = 0; i < trackedDevices.size(); i++) {
        if (trackedDevices[i].mac.equals(mac)) {
            return i;
        }
    }
    return -1;
}

void tracking_update(const std::vector<Device>& wifiDevices, 
                     const std::vector<Device>& btDevices) {
    
    unsigned long currentTime = millis();
    
    // Mark all devices as not seen this cycle
    for (auto& dev : trackedDevices) {
        dev.isNew = false;
    }
    
    // Process WiFi devices
    for (const auto& dev : wifiDevices) {
        int idx = findDeviceIndex(dev.mac);
        
        if (idx >= 0) {
            // Update existing device
            TrackedDevice& tracked = trackedDevices[idx];
            tracked.name = dev.name;
            tracked.rssi = dev.rssi;
            tracked.distance = dev.distance;
            tracked.channel = dev.channel;
            tracked.lastSeen = currentTime;
            tracked.seenCount++;
            
            // Update average RSSI for better distance estimation
            tracked.avgRSSI = (tracked.avgRSSI * (tracked.seenCount - 1) + dev.rssi) / tracked.seenCount;
            
        } else {
            // New device found
            TrackedDevice newDevice;
            newDevice.mac = dev.mac;
            newDevice.name = dev.name;
            newDevice.rssi = dev.rssi;
            newDevice.avgRSSI = dev.rssi;
            newDevice.distance = dev.distance;
            newDevice.type = dev.type;
            newDevice.channel = dev.channel;
            newDevice.lastSeen = currentTime;
            newDevice.firstSeen = currentTime;
            newDevice.seenCount = 1;
            newDevice.isNew = true;
            
            trackedDevices.push_back(newDevice);
            
            Serial.printf("[NEW] %s | %s | %.1fm\n", 
                         dev.type == TYPE_WIFI_AP ? "WiFi AP" : "WiFi",
                         dev.name.isEmpty() ? dev.mac.c_str() : dev.name.c_str(),
                         dev.distance);
        }
    }
    
    // Process Bluetooth devices
    for (const auto& dev : btDevices) {
        int idx = findDeviceIndex(dev.mac);
        
        if (idx >= 0) {
            // Update existing device
            TrackedDevice& tracked = trackedDevices[idx];
            tracked.name = dev.name;
            tracked.rssi = dev.rssi;
            tracked.distance = dev.distance;
            tracked.lastSeen = currentTime;
            tracked.seenCount++;
            tracked.avgRSSI = (tracked.avgRSSI * (tracked.seenCount - 1) + dev.rssi) / tracked.seenCount;
            
        } else {
            // New device found
            TrackedDevice newDevice;
            newDevice.mac = dev.mac;
            newDevice.name = dev.name;
            newDevice.rssi = dev.rssi;
            newDevice.avgRSSI = dev.rssi;
            newDevice.distance = dev.distance;
            newDevice.type = dev.type;
            newDevice.channel = 0;
            newDevice.lastSeen = currentTime;
            newDevice.firstSeen = currentTime;
            newDevice.seenCount = 1;
            newDevice.isNew = true;
            
            trackedDevices.push_back(newDevice);
            
            Serial.printf("[NEW] BLE | %s | %.1fm\n", 
                         dev.name.c_str(), dev.distance);
        }
    }
    
    // Remove devices that haven't been seen recently
    trackedDevices.erase(
        std::remove_if(trackedDevices.begin(), trackedDevices.end(),
            [currentTime](const TrackedDevice& dev) {
                bool timeout = (currentTime - dev.lastSeen) > DEVICE_TIMEOUT;
                if (timeout) {
                    Serial.printf("[LOST] %s (%s)\n", 
                                 dev.name.c_str(), dev.mac.c_str());
                }
                return timeout;
            }),
        trackedDevices.end()
    );
    
    lastUpdateTime = currentTime;
}

std::vector<TrackedDevice> tracking_getAllDevices() {
    return trackedDevices;
}

std::vector<TrackedDevice> tracking_getDevicesByType(DeviceType type) {
    std::vector<TrackedDevice> filtered;
    
    for (const auto& dev : trackedDevices) {
        if (dev.type == type) {
            filtered.push_back(dev);
        }
    }
    
    return filtered;
}

std::vector<TrackedDevice> tracking_getNearbyDevices(float maxDistance) {
    std::vector<TrackedDevice> nearby;
    
    for (const auto& dev : trackedDevices) {
        if (dev.distance <= maxDistance) {
            nearby.push_back(dev);
        }
    }
    
    // Sort by distance (closest first)
    std::sort(nearby.begin(), nearby.end(),
        [](const TrackedDevice& a, const TrackedDevice& b) {
            return a.distance < b.distance;
        });
    
    return nearby;
}

TrackedDevice* tracking_getDeviceByMAC(const String& mac) {
    int idx = findDeviceIndex(mac);
    if (idx >= 0) {
        return &trackedDevices[idx];
    }
    return nullptr;
}

int tracking_getDeviceCount() {
    return trackedDevices.size();
}

void tracking_clear() {
    trackedDevices.clear();
    Serial.println("All tracked devices cleared");
}

// Get statistics
void tracking_printStats() {
    Serial.println("\n=== Device Tracking Statistics ===");
    Serial.printf("Total devices: %d\n", trackedDevices.size());
    
    int wifiCount = 0, bleCount = 0, clientCount = 0;
    
    for (const auto& dev : trackedDevices) {
        if (dev.type == TYPE_WIFI_AP) wifiCount++;
        else if (dev.type == TYPE_BLUETOOTH) bleCount++;
        else clientCount++;
    }
    
    Serial.printf("WiFi APs: %d\n", wifiCount);
    Serial.printf("BLE Devices: %d\n", bleCount);
    Serial.printf("WiFi Clients: %d\n", clientCount);
    
    // Find closest device
    if (!trackedDevices.empty()) {
        auto closest = std::min_element(trackedDevices.begin(), trackedDevices.end(),
            [](const TrackedDevice& a, const TrackedDevice& b) {
                return a.distance < b.distance;
            });
        
        Serial.printf("\nClosest device: %s (%.2fm)\n", 
                     closest->name.c_str(), closest->distance);
    }
    
    Serial.println("==================================\n");
}