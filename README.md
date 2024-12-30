# ESP32-LoRa-Chat
A web-based chat application using ESP32 and LoRa communication for long-range wireless messaging.

## 🌟 Features
- 💬 Real-time messaging over LoRa
- 🌐 Web-based interface accessible via WiFi
- 📡 Long-range communication using SX1278 LoRa Module Ra-02
- 📱 Mobile-friendly responsive design
- 🔄 Automatic message synchronization
- 📊 Message history storage
- 🚦 Connection status indicators
- ⚡ Easy setup and configuration

## 📋 Hardware Requirements
- ESP32 Development Board
- SX1278 LoRa Module Ra-02
- 433MHz Spring Antenna
  - Center frequency: 433MHz
  - Gain: 3dBi
  - Input impedance: 50 ohm
  - Interface: IPX1

## 🔌 Wire Connections
```
ESP32 -> LoRa Module
- GPIO5  -> NSS/CS
- GPIO18 -> SCK
- GPIO19 -> MISO
- GPIO23 -> MOSI
```

## 🚀 Setup Instructions
1. Install Required Libraries:
   - WiFi.h
   - WebServer.h
   - ArduinoJson
   - SPI.h
   - LoRa.h
   - SPIFFS.h

2. Upload Code:
   - Download the code from this repository
   - Open in Arduino IDE
   - Select ESP32 board
   - Upload to two ESP32 devices
   - For second device, change LoRa addresses in code:
     ```cpp
     // First ESP32
     byte localAddress = 0xAA;    
     byte destination = 0xBB;     

     // Second ESP32
     byte localAddress = 0xBB;    
     byte destination = 0xAA;     
     ```

3. Connect:
   - Network name: ESP32-LoRa-Chat
   - Password: chatpass123
   - Web interface: 192.168.4.1


## 💻 Usage
1. Power on both ESP32 devices
2. Connect to either ESP32's WiFi network
3. Open web browser and go to 192.168.4.1
4. Enter username in the prompt
5. Start chatting!

## 📡 Hardware Specifications

### LoRa Module (SX1278 Ra-02)
- Frequency: 433MHz
- Spreading Factor: 10
- Bandwidth: 125kHz
- Coding Rate: 4/5
- Maximum Range: Several kilometers (line of sight)

### Antenna Specifications
- Type: Spring Antenna
- Center Frequency: 433MHz
- VSWR: ≤1.5
- Gain: 3dBi
- Maximum Power: 10W
- Input Impedance: 50 ohm
- Interface: IPX1

## 🤝 Contributing
Contributions are welcome! Please feel free to submit a Pull Request.

## 📄 License
This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 📞 Contact
- GitHub: [@Skca01](https://github.com/Skca01)
- Project Link: [https://github.com/Skca01/ESP32-LoRa-Chat](https://github.com/Skca01/ESP32-LoRa-Chat)

## 🙏 Acknowledgments
- ESP32 Community
- LoRa Development Community
- Contributors and Testers

---
