#pragma once
#include <vector>
#include <Arduino.h>
#include <WiFi.h>

using namespace std;

enum DeviceType {
    TYPE_WIFI_AP,
    TYPE_WIFI_CLIENT,
    TYPE_BLUETOOTH
};

struct Device {
    String mac;
    String name;
    int rssi;
    float distance;
    DeviceType type;
    uint8_t channel;
    wifi_auth_mode_t encryption;
};

void wifi_init();
std::vector<Device> wifi_scan();

// Packet sniffing
void wifi_enable_promiscuous();
void wifi_disable_promiscuous();
void wifi_set_channel(uint8_t channel);

// Utility functions
bool wifi_isOpenNetwork(const Device& device);
String wifi_getEncryptionType(wifi_auth_mode_t encType);
bool wifi_tryConnect(const String& ssid, const String& password = "", int timeout = 10000);