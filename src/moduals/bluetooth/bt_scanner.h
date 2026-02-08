#pragma once

#include <vector>
#include "../wifi/wifi_scanner.h"

void bt_init();
std::vector<Device> bt_scan();

// Connection functions
bool bt_connect(const String& address);
void bt_disconnect();
void bt_getDeviceInfo();