#include "wifi_scanner.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include "../utils/distance.h"

using namespace std;

// External packet counter (increment from main)
extern void incrementPacketCount();

// Packet sniffing callback
static void wifi_sniffer_callback(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT) return;
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    wifi_pkt_rx_ctrl_t ctrl = pkt->rx_ctrl;
    
    // Increment global packet counter
    incrementPacketCount();
    
    // Get packet data
    const uint8_t* payload = pkt->payload;
    int len = ctrl.sig_len;
    
    // Parse MAC addresses from management frame
    if (len >= 24) {
        uint8_t* srcMAC = (uint8_t*)(payload + 10);  // Source address
        uint8_t* dstMAC = (uint8_t*)(payload + 4);   // Destination address
        
        // Log interesting packets (not broadcast)
        if (dstMAC[0] != 0xFF) {
            char macStr[18];
            sprintf(macStr, "%02X:%02X:%02X:%02X:%02X:%02X",
                    srcMAC[0], srcMAC[1], srcMAC[2], srcMAC[3], srcMAC[4], srcMAC[5]);
            
            Serial.printf("[SNIFF] PKT from %s | RSSI: %d | CH: %d\n", 
                         macStr, ctrl.rssi, ctrl.channel);
        }
    }
}

void wifi_init() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    Serial.println("WiFi initialized in Station mode");
}

void wifi_enable_promiscuous() {
    // Enable promiscuous mode for packet sniffing
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_promiscuous_rx_cb(&wifi_sniffer_callback);
    Serial.println("Promiscuous mode enabled - Packet sniffing active");
}

void wifi_disable_promiscuous() {
    esp_wifi_set_promiscuous(false);
    Serial.println("Promiscuous mode disabled");
}

void wifi_set_channel(uint8_t channel) {
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
}

std::vector<Device> wifi_scan() {
    // Disable promiscuous mode during scan
    bool wasPromiscuous = false;
    esp_wifi_get_promiscuous(&wasPromiscuous);
    if (wasPromiscuous) {
        wifi_disable_promiscuous();
    }
    
    std::vector<Device> list;

    // Scan for networks (hidden networks included)
    int n = WiFi.scanNetworks(false, true, false, 300);

    for (int i = 0; i < n; i++) {
        Device d;
        d.mac = WiFi.BSSIDstr(i);
        d.name = WiFi.SSID(i);
        d.rssi = WiFi.RSSI(i);
        
        // Improved distance estimation for WiFi
        // Use different TX power and path loss model for better accuracy
        d.distance = estimateDistanceWiFi_Enhanced(d.rssi);
        
        d.type = TYPE_WIFI_AP;
        d.channel = WiFi.channel(i);
        d.encryption = WiFi.encryptionType(i);

        list.push_back(d);
    }
    
    // Re-enable promiscuous mode if it was on
    if (wasPromiscuous) {
        wifi_enable_promiscuous();
    }

    return list;
}

bool wifi_isOpenNetwork(const Device& device) {
    return device.encryption == WIFI_AUTH_OPEN;
}

String wifi_getEncryptionType(wifi_auth_mode_t encType) {
    switch(encType) {
        case WIFI_AUTH_OPEN:            return "Open";
        case WIFI_AUTH_WEP:             return "WEP";
        case WIFI_AUTH_WPA_PSK:         return "WPA";
        case WIFI_AUTH_WPA2_PSK:        return "WPA2";
        case WIFI_AUTH_WPA_WPA2_PSK:    return "WPA/WPA2";
        case WIFI_AUTH_WPA2_ENTERPRISE: return "WPA2-E";
        case WIFI_AUTH_WPA3_PSK:        return "WPA3";
        default:                        return "Unknown";
    }
}

// Advanced: Try to connect to a specific network
bool wifi_tryConnect(const String& ssid, const String& password, int timeout) {
    Serial.printf("Attempting to connect to: %s\n", ssid.c_str());
    
    WiFi.begin(ssid.c_str(), password.c_str());
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < timeout) {
        delay(500);
        Serial.print(".");
    }
    Serial.println();
    
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("Connected successfully!");
        Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
        return true;
    } else {
        Serial.println("Connection failed");
        WiFi.disconnect();
        return false;
    }
}