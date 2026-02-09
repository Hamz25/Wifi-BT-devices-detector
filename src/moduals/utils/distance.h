#pragma once
#include <Arduino.h>

// ============ IMPROVED DISTANCE ESTIMATION ============
// Uses multiple calibrated models for better accuracy

// Enhanced distance estimation using path loss model
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
    
    // Clamp RSSI to reasonable values
    if (rssi > 0) rssi = -1;
    if (rssi < -100) rssi = -100;
    
    float ratio = (txPower - rssi) / (10.0 * environmentFactor);
    float distance = pow(10.0, ratio);
    
    return distance;
}

// ============ ENHANCED WiFi DISTANCE ESTIMATION ============
// More accurate model for 2.4 GHz WiFi with real-world calibration
inline float estimateDistanceWiFi_Enhanced(int rssi) {
    // WiFi typically has higher TX power than BLE
    // Calibrated values based on common routers
    
    if (rssi >= -30) {
        // Very close range (< 1m) - use linear approximation
        return 0.5;
    } else if (rssi >= -50) {
        // Close range (1-3m) - use measured power of -40 at 1m
        return estimateDistance(rssi, -40, 2.2);
    } else if (rssi >= -70) {
        // Medium range (3-15m) - indoor with some obstacles
        return estimateDistance(rssi, -45, 2.7);
    } else {
        // Far range (15m+) - heavy attenuation
        return estimateDistance(rssi, -50, 3.5);
    }
}

// ============ ENHANCED BLE DISTANCE ESTIMATION ============
// More accurate model for Bluetooth Low Energy with real-world calibration
inline float estimateDistanceBLE_Enhanced(int rssi) {
    // BLE typically has lower TX power
    // Calibrated for common BLE devices (phones, headphones, trackers)
    
    if (rssi >= -35) {
        // Very close range (< 0.5m)
        return 0.3;
    } else if (rssi >= -59) {
        // Close range (0.5-2m) - standard BLE measured power
        return estimateDistance(rssi, -59, 2.0);
    } else if (rssi >= -75) {
        // Medium range (2-10m)
        return estimateDistance(rssi, -62, 2.5);
    } else {
        // Far range (10m+)
        return estimateDistance(rssi, -65, 3.2);
    }
}

// Alternative: Simple estimation for WiFi (2.4 GHz) - Legacy function
inline float estimateDistanceWiFi(int rssi) {
    // WiFi typically has higher TX power than BLE
    return estimateDistance(rssi, -50, 2.5);
}

// Alternative: Simple estimation for Bluetooth LE - Legacy function
inline float estimateDistanceBLE(int rssi) {
    // BLE typically has lower TX power
    return estimateDistance(rssi, -59, 2.0);
}

// ============ ADVANCED: KALMAN FILTER FOR DISTANCE SMOOTHING ============
// Reduces noise in distance measurements
class DistanceFilter {
private:
    float estimate;
    float errorEstimate;
    float measurementNoise;
    float processNoise;
    bool initialized;
    
public:
    DistanceFilter(float initialEstimate = 5.0, 
                   float measurementNoise = 1.0, 
                   float processNoise = 0.1) 
        : estimate(initialEstimate)
        , errorEstimate(1.0)
        , measurementNoise(measurementNoise)
        , processNoise(processNoise)
        , initialized(false) {}
    
    float update(float measurement) {
        if (!initialized) {
            estimate = measurement;
            initialized = true;
            return estimate;
        }
        
        // Prediction step
        errorEstimate += processNoise;
        
        // Update step
        float kalmanGain = errorEstimate / (errorEstimate + measurementNoise);
        estimate += kalmanGain * (measurement - estimate);
        errorEstimate = (1.0 - kalmanGain) * errorEstimate;
        
        return estimate;
    }
    
    float getEstimate() const {
        return estimate;
    }
    
    void reset() {
        initialized = false;
        errorEstimate = 1.0;
    }
};

// ============ DISTANCE CATEGORIZATION ============
// Convert distance to proximity category
inline String getProximityCategory(float distance) {
    if (distance < 0) return "Unknown";
    if (distance < 1.0) return "Immediate";
    if (distance < 3.0) return "Near";
    if (distance < 10.0) return "Medium";
    return "Far";
}

// More granular proximity levels
inline int getProximityLevel(float distance) {
    // Returns 0-5 (0 = unknown, 1 = immediate, 5 = very far)
    if (distance < 0) return 0;
    if (distance < 0.5) return 1;
    if (distance < 2.0) return 2;
    if (distance < 5.0) return 3;
    if (distance < 15.0) return 4;
    return 5;
}

// ============ SIGNAL QUALITY INDICATORS ============
inline String getSignalQuality(int rssi) {
    if (rssi >= -50) return "Excellent";
    if (rssi >= -60) return "Good";
    if (rssi >= -70) return "Fair";
    if (rssi >= -80) return "Weak";
    return "Very Weak";
}

// Get signal strength as percentage (0-100)
inline int getSignalStrengthPercent(int rssi) {
    // Convert RSSI to percentage
    // -30 dBm = 100%, -90 dBm = 0%
    if (rssi >= -30) return 100;
    if (rssi <= -90) return 0;
    
    return (int)(2 * (rssi + 100));
}

// Get signal bars (1-5)
inline int getSignalBars(int rssi) {
    if (rssi >= -50) return 5;
    if (rssi >= -60) return 4;
    if (rssi >= -70) return 3;
    if (rssi >= -80) return 2;
    if (rssi >= -90) return 1;
    return 0;
}

// ============ PATH LOSS ANALYSIS ============
// Estimate path loss exponent from multiple RSSI measurements
inline float estimatePathLoss(float distance1, int rssi1, float distance2, int rssi2) {
    if (distance1 <= 0 || distance2 <= 0 || distance1 == distance2) {
        return 2.0; // Default free space
    }
    
    float pathLoss = (rssi1 - rssi2) / (10.0 * log10(distance2 / distance1));
    
    // Clamp to reasonable values
    if (pathLoss < 1.5) pathLoss = 1.5;
    if (pathLoss > 5.0) pathLoss = 5.0;
    
    return pathLoss;
}