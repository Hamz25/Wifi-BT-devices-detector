You need to Install Arduino-cli to run this project on VScode

##Bash (Arch)

sudo pacman -S arduino-cli
arduino-cli core update-index
arduino-cli core install esp32:esp32

2- Connect VScode 

create a folder named .vscode in your project root

inside it create this JSON file and name it tasks.json

paste this inside :

{
    "version": "2.0.0",
    "tasks": [
        {
            "label": "ESP32: Build",
            "type": "shell",
            "command": "arduino-cli compile --fqbn esp32:esp32:esp32 ${workspaceFolder}",
            "group": { "kind": "build", "isDefault": true }
        },
        {
            "label": "ESP32: Upload",
            "type": "shell",
            "command": "arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32 ${workspaceFolder}",
            "problemMatcher": []
        }
    ]
}

You might need to change /dev/ttyUSB0 to match your specific port

To Compile: Press Ctrl + Shift + B.

To Upload/Test: Press Ctrl + Shift + P, type "Run Task", and select ESP32: Upload.

To see Serial Output: Use the built-in VS Code terminal and run: arduino-cli monitor -p /dev/ttyUSB0


to use c++ in your ESP-32 you need to install ESP-IDF

First, install the necessary build tools and dependencies:

Bash
sudo pacman -S --needed gcc git make cmake gperf python-pip ninja dfu-util libusb

Download and Install the ESP-IDF
Clone the framework into a folder

mkdir -p ~/esp #creates a folder named esp
cd ~/esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh esp32  # This installs the compilers for the ESP32