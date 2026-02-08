# WiFi/BT Devices Detector

A simple ESP32-S3 based project that scans and detects nearby WiFi and Bluetooth devices, displaying real-time information on a small screen.

## 📋 Overview

This project uses an ESP32-S3 microcontroller to capture and monitor WiFi and Bluetooth (BT) devices in the surrounding area. The detected devices are displayed on a compact screen, making it useful for network analysis, security monitoring, or general wireless device tracking.

## ✨ Features

- **WiFi Device Detection**: Scans for nearby WiFi access points and client devices
- **Bluetooth Device Scanning**: Detects BLE (Bluetooth Low Energy) and classic Bluetooth devices
- **Real-time Display**: Shows device information on a small screen
- **Lightweight & Portable**: Compact design suitable for portable use
- **ESP32-S3 Powered**: Leverages the dual-core ESP32-S3 with built-in WiFi and Bluetooth capabilities

## 🛠️ Hardware Requirements

- **ESP32-S3 Development Board** (e.g., ESP32-S3-DevKitC-1)
- **Small Display** (OLED/LCD)
- **USB Cable** for programming and power
- **Optional**: External antenna for improved range

## 📦 Software Requirements

- [PlatformIO](https://platformio.org/) IDE or CLI
- ESP32 board support package
- Required libraries (automatically handled by PlatformIO)

## 🚀 Getting Started

### Installation

1. **Clone the repository**
   ```bash
   git clone https://github.com/Hamz25/Wifi-BT-devices-detector.git
   cd Wifi-BT-devices-detector
   ```

2. **Open with PlatformIO**
   - Open the project folder in VS Code with PlatformIO extension installed
   - Or use PlatformIO CLI

3. **Build the project**
   ```bash
   pio run
   ```

4. **Upload to ESP32-S3**
   ```bash
   pio run --target upload
   ```

5. **Monitor serial output** (optional)
   ```bash
   pio device monitor
   ```

### Configuration

Check the `platformio.ini` file for board configuration and build settings. The project is pre-configured for ESP32-S3 development boards.

## 📁 Project Structure

```
Wifi-BT-devices-detector/
├── .vscode/              # VS Code configuration
├── src/
│   └── moduals/         # Source code modules
├── platformio.ini       # PlatformIO configuration
├── .gitignore
└── README.md
```

## 🔧 Usage

Once uploaded to your ESP32-S3:

1. Power on the device
2. The screen will initialize and start scanning
3. WiFi networks and Bluetooth devices will be detected automatically
4. Information displayed typically includes:
   - Device name/SSID
   - MAC address
   - Signal strength (RSSI)
   - Device type (WiFi/BT)

## 📊 Display Information

The device displays real-time information about detected wireless devices. Check the DOCS folder for detailed information about the display format and available data fields.

## 🔍 Technical Details

- **Microcontroller**: ESP32-S3 (dual-core Xtensa LX7)
- **WiFi**: 802.11 b/g/n (2.4 GHz)
- **Bluetooth**: BLE 5.0 and Classic Bluetooth
- **Framework**: Arduino/ESP-IDF via PlatformIO
- **Programming Language**: C++

## ⚠️ Legal Disclaimer

This project is intended for **educational and security research purposes only**. Users are responsible for ensuring compliance with local laws and regulations regarding wireless monitoring. Unauthorized monitoring of wireless communications may be illegal in your jurisdiction.

- Only scan networks and devices you own or have permission to monitor
- Respect privacy and security of others
- Do not use for malicious purposes

## 🤝 Contributing

Contributions are welcome! Here's how you can help:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/amazing-feature`)
3. Commit your changes (`git commit -m 'Add some amazing feature'`)
4. Push to the branch (`git push origin feature/amazing-feature`)
5. Open a Pull Request

## 📝 License

This project is open source. Please check the repository for license information.

## 🐛 Issues & Support

If you encounter any issues or have questions:

- Open an issue on the [GitHub Issues](https://github.com/Hamz25/Wifi-BT-devices-detector/issues) page
- Provide detailed information about your hardware and the problem

## 🙏 Acknowledgments

- ESP32 community for extensive documentation and examples
- PlatformIO for the excellent development platform
- All contributors and users of this project

## 📚 Resources

- [ESP32-S3 Documentation](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/)
- [PlatformIO Documentation](https://docs.platformio.org/)
- [ESP32 Arduino Core](https://github.com/espressif/arduino-esp32)

---
