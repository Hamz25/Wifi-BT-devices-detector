#include "bt_scanner.h"
#include <BLEDevice.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include "../utils/distance.h"

BLEScan* scanner;
BLEClient* bleClient = nullptr;

// Device class identification based on appearance or services
String identifyDeviceType(BLEAdvertisedDevice device) {
    // Check appearance value
    if (device.haveAppearance()) {
        uint16_t appearance = device.getAppearance();
        
        // Standard Bluetooth appearance values
        if (appearance >= 832 && appearance <= 895) return "Headphones/Earbuds";
        if (appearance >= 896 && appearance <= 959) return "Speaker";
        if (appearance >= 960 && appearance <= 1023) return "Headset";
        if (appearance >= 1024 && appearance <= 1087) return "Keyboard";
        if (appearance >= 1088 && appearance <= 1151) return "Mouse";
        if (appearance >= 1152 && appearance <= 1215) return "Gamepad";
        if (appearance == 576) return "Watch";
        if (appearance == 577) return "Fitness Tracker";
        if (appearance >= 704 && appearance <= 767) return "Display";
        if (appearance >= 256 && appearance <= 319) return "Phone";
    }
    
    // Check service UUIDs
    if (device.haveServiceUUID()) {
        // Common service UUIDs
        if (device.isAdvertisingService(BLEUUID("110B"))) return "Audio Source";
        if (device.isAdvertisingService(BLEUUID("110A"))) return "Audio Sink";
        if (device.isAdvertisingService(BLEUUID("180F"))) return "Battery Service";
        if (device.isAdvertisingService(BLEUUID("1812"))) return "HID Device";
        if (device.isAdvertisingService(BLEUUID("180A"))) return "Device Info";
        if (device.isAdvertisingService(BLEUUID("181C"))) return "Fitness Device";
    }
    
    // Check manufacturer data for known vendors
    if (device.haveManufacturerData()) {
        std::string mfgData = device.getManufacturerData();
        if (mfgData.length() >= 2) {
            uint16_t companyId = (mfgData[1] << 8) | mfgData[0];
            
            switch(companyId) {
                case 0x004C: return "Apple Device";
                case 0x0075: return "Samsung Device";
                case 0x00E0: return "Google Device";
                case 0x0006: return "Microsoft Device";
                case 0x0087: return "Garmin Device";
                case 0x0157: return "Bose Device";
                case 0x00A8: return "Sony Device";
                default: break;
            }
        }
    }
    
    return "Unknown BLE";
}

// Check if device is likely a headphone/audio device
bool isAudioDevice(BLEAdvertisedDevice device) {
    String type = identifyDeviceType(device);
    return type.indexOf("Headphones") >= 0 || 
           type.indexOf("Speaker") >= 0 || 
           type.indexOf("Headset") >= 0 ||
           type.indexOf("Audio") >= 0 ||
           type.indexOf("Earbuds") >= 0;
}

void bt_init() {
    BLEDevice::init("ESP32-Tracker");
    scanner = BLEDevice::getScan();
    scanner->setActiveScan(true);
    scanner->setInterval(100);
    scanner->setWindow(99);
    
    Serial.println("Bluetooth initialized");
}

std::vector<Device> bt_scan() {
    std::vector<Device> list;

    BLEScanResults results = scanner->start(3, false);

    for (int i = 0; i < results.getCount(); i++) {
        BLEAdvertisedDevice dev = results.getDevice(i);

        Device d;
        d.mac = dev.getAddress().toString().c_str();
        d.name = dev.haveName() ? dev.getName().c_str() : identifyDeviceType(dev);
        d.rssi = dev.getRSSI();
        
        // Enhanced distance estimation for BLE
        d.distance = estimateDistanceBLE_Enhanced(d.rssi);
        
        d.type = TYPE_BLUETOOTH;
        d.channel = 0; // BLE uses adaptive frequency hopping
        
        list.push_back(d);
        
        // Log interesting devices
        if (isAudioDevice(dev)) {
            Serial.printf("[BT] Audio device found: %s (%s) | %.2fm\n", 
                         d.name.c_str(), d.mac.c_str(), d.distance);
        }
    }
    
    scanner->clearResults();
    return list;
}

// Try to connect to a BLE device (for audio devices like headphones)
bool bt_connect(const String& address) {
    Serial.printf("Attempting BLE connection to: %s\n", address.c_str());
    
    BLEAddress bleAddress(address.c_str());
    
    if (bleClient == nullptr) {
        bleClient = BLEDevice::createClient();
    }
    
    // Try to connect
    if (bleClient->connect(bleAddress)) {
        Serial.println("Connected to BLE device!");
        
        // List available services (services are auto-discovered on connect)
        std::map<std::string, BLERemoteService*>* services = bleClient->getServices();
        Serial.printf("Found %d services:\n", services->size());
        
        for (auto &service : *services) {
            Serial.printf("  Service: %s\n", service.first.c_str());
        }
        
        return true;
    } else {
        Serial.println("Connection failed");
        return false;
    }
}

void bt_disconnect() {
    if (bleClient != nullptr && bleClient->isConnected()) {
        bleClient->disconnect();
        Serial.println("Disconnected from BLE device");
    }
}

// Get detailed information about a connected device
void bt_getDeviceInfo() {
    if (bleClient == nullptr || !bleClient->isConnected()) {
        Serial.println("No device connected");
        return;
    }
    
    Serial.println("\n=== Connected Device Info ===");
    
    // Try to read Device Information Service (0x180A)
    BLERemoteService* infoService = bleClient->getService(BLEUUID((uint16_t)0x180A));
    
    if (infoService != nullptr) {
        // Manufacturer Name (0x2A29)
        BLERemoteCharacteristic* mfgChar = infoService->getCharacteristic(BLEUUID((uint16_t)0x2A29));
        if (mfgChar && mfgChar->canRead()) {
            std::string value = mfgChar->readValue();
            Serial.printf("Manufacturer: %s\n", value.c_str());
        }
        
        // Model Number (0x2A24)
        BLERemoteCharacteristic* modelChar = infoService->getCharacteristic(BLEUUID((uint16_t)0x2A24));
        if (modelChar && modelChar->canRead()) {
            std::string value = modelChar->readValue();
            Serial.printf("Model: %s\n", value.c_str());
        }
        
        // Serial Number (0x2A25)
        BLERemoteCharacteristic* serialChar = infoService->getCharacteristic(BLEUUID((uint16_t)0x2A25));
        if (serialChar && serialChar->canRead()) {
            std::string value = serialChar->readValue();
            Serial.printf("Serial: %s\n", value.c_str());
        }
    }
    
    // Try to read Battery Service (0x180F)
    BLERemoteService* batteryService = bleClient->getService(BLEUUID((uint16_t)0x180F));
    
    if (batteryService != nullptr) {
        BLERemoteCharacteristic* batteryChar = batteryService->getCharacteristic(BLEUUID((uint16_t)0x2A19));
        if (batteryChar && batteryChar->canRead()) {
            uint8_t value = batteryChar->readUInt8();
            Serial.printf("Battery: %d%%\n", value);
        }
    }
    
    Serial.println("=============================\n");
}