#pragma once
#include <Arduino.h>

// Improved distance estimation using path loss model
// Formula: RSSI = TxPower - 10 * n * log10(distance)
// Solved for distance: distance = 10 ^ ((TxPower - RSSI) / (10 * n))

inline float estimateDistance(int rssi, int txPower = -59, float environmentFactor = 2.0) {
    // txPower: Typical transmission power at 1 meter (usually -59 to -55 dBm)
    // environmentFactor (n): 
    //   - 2.0 = Free space (ideal)
    //   - 2.5 = Indoor (typical)
    //   - 3.0 = Indoor with obstacles
    //   - 4.0 = Dense indoor
    
    if (rssi == 0) {
        return -1.0; // Invalid reading
    }
    
    float ratio = (txPower - rssi) / (10.0 * environmentFactor);
    float distance = pow(10.0, ratio);
    
    return distance;
}

// Alternative: Simple estimation for WiFi (2.4 GHz)
inline float estimateDistanceWiFi(int rssi) {
    // WiFi typically has higher TX power than BLE
    return estimateDistance(rssi, -50, 2.5);
}

// Alternative: Simple estimation for Bluetooth LE
inline float estimateDistanceBLE(int rssi) {
    // BLE typically has lower TX power
    return estimateDistance(rssi, -59, 2.0);
}

// Convert distance to proximity category
inline String getProximityCategory(float distance) {
    if (distance < 0) return "Unknown";
    if (distance < 1.0) return "Immediate";
    if (distance < 3.0) return "Near";
    if (distance < 10.0) return "Medium";
    return "Far";
}

// Signal quality indicator
inline String getSignalQuality(int rssi) {
    if (rssi >= -50) return "Excellent";
    if (rssi >= -60) return "Good";
    if (rssi >= -70) return "Fair";
    if (rssi >= -80) return "Weak";
    return "Very Weak";
}