# Geiger Counter (RadiationD-v1.1 CAJOE)

[![MIT License](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![GitHub stars](https://img.shields.io/github/stars/SensorsIot/Geiger-Counter-RadiationD-v1.1-CAJOE-)](https://github.com/SensorsIot/Geiger-Counter-RadiationD-v1.1-CAJOE-/stargazers)
[![YouTube](https://img.shields.io/badge/YouTube-Video-red?logo=youtube)](https://youtu.be/K28Az3-gV7E)

![ESP32](https://img.shields.io/badge/ESP32-Supported-green?logo=espressif)
![Arduino](https://img.shields.io/badge/Arduino-Compatible-00979D?logo=arduino)
![Radiation](https://img.shields.io/badge/CPM-Counts%20Per%20Minute-orange)

Interface with the RadiationD-v1.1 (CAJOE) Geiger counter board. Displays readings on OLED, sends data to ThingSpeak, and triggers IFTTT alerts.

📺 **Video Tutorial:** https://youtu.be/K28Az3-gV7E

## 🔌 Wiring

| Function | GPIO |
|----------|------|
| Geiger Counter Output | GPIO 26 |
| OLED SDA | GPIO 5 |
| OLED SCL | GPIO 4 |

## 📦 Dependencies

Install using the Arduino Library Manager:

| Library | Description |
|---------|-------------|
| [ThingPulse SSD1306](https://github.com/ThingPulse/esp8266-oled-ssd1306) | OLED display driver |
| [IFTTTWebhook](https://github.com/romkey/IFTTTWebhook) | IFTTT notifications |
| [ThingSpeak](https://github.com/mathworks/thingspeak-arduino) | ThingSpeak API |

## ⚙️ Configuration

Edit the credentials in `Geiger_Counter.ino`:

```cpp
// WiFi
#define mySSID "your-wifi-name"
#define myPASSWORD "your-wifi-password"

// IFTTT
#define IFTTT_KEY "your-ifttt-key"

// ThingSpeak
#define SECRET_CH_ID 0000000           // Your channel number
#define SECRET_WRITE_APIKEY "xxx"       // Your API key
```

### ⏱️ Timing Settings

```cpp
#define PERIOD_LOG 15          // Display update interval (seconds)
#define PERIOD_THINKSPEAK 3600 // ThingSpeak upload interval (seconds)
```

## 📊 Output

- **Display:** CPM (Counts Per Minute)
- **ThingSpeak:** CPH (Counts Per Hour)
- **IFTTT Alert:** Triggered when CPM ≥ 200

