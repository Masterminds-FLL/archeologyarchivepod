# 🧊 Archeology Archive – Preservation Pod

A portable, climate-controlled system designed to preserve archaeological artifacts **and their context** from dig site to lab.

---

## 📌 Overview

The Preservation Pod solves a critical problem in archaeology:

> Once an artifact is uncovered, its context begins to disappear.

This system:
- Maintains **temperature, humidity, and light conditions**
- Protects fragile artifacts during transport
- Records environmental data to an **SD card**
- Links physical artifacts to digital records via **QR codes**

Part of the larger **Archaeology Archive system** (App + AI + QR + Pod).

---

## 🧰 Bill of Materials

### 🧱 Structure
- Insulated cooler (15–30 qt recommended)
- Backpack straps (optional but recommended)
- Internal mounting plate (3D printed or acrylic)

---

### 🔌 Electronics

| Component | Description |
|----------|-------------|
| ESP32 (or Arduino) | Main controller |
| SHT31 | Temperature + humidity sensor |
| VEML7700 | Light sensor |
| (Optional) VEML6075 | UV sensor |
| Micro SD module | Data logging |
| LCD display (I2C) | Real-time readout |
| Fan (5V or 12V) | Air circulation |
| MOSFET module | Fan control |
| 12V battery (LiFePO4 recommended) | Power supply |
| Buck converter | 12V → 5V |

---

### 🧪 Protection Materials
- Cotton batting (recommended for fragile artifacts)
- Foam inserts (custom or 3D printed)
- Optional:
  - Aluminum foil (for delicate materials)
  - Small containers / pill bottles

---

### 🔧 Misc
- Jumper wires
- Breadboard or soldered PCB
- On/off switch

---

## 🧱 3D Printed Parts

STL files are located in this repo.

Typical parts:
- Sensor mounts
- Internal tray system
- Fan housing
- Cable guides

**Recommended material:** PETG (PLA also works)

---

## 🔧 Assembly Instructions

### 1. Prepare the Cooler
- Clean interior
- Drill small holes for:
  - Sensors
  - Wiring
- Install mounting plate

---

### 2. Install Electronics
- Mount ESP32 / Arduino
- Add SD module + MOSFET
- Secure battery

---

### 3. Install Sensors
- Place centrally for accurate readings
- Avoid direct airflow from fan

---

### 4. Install Fan
- Mount on side or lid
- Connect via MOSFET

---

### 5. Install Display
- Mount LCD externally or on lid
- Connect via I2C

---

### 6. Build Interior Storage
- Add foam compartments or trays
- Line with cotton batting

**Goal:** Prevent movement + preserve environment

---

### 7. Add Carry System
- Install straps or handles
- Keep total weight ~15–25 lbs

---

## 💻 Software Setup

### 📦 Required Libraries

Install in Arduino IDE:

- Adafruit SHT31
- Adafruit VEML7700
- SD
- Wire
- LiquidCrystal_I2C

---

### 🧠 Core Functionality

The system:
1. Reads sensor data
2. Displays real-time values
3. Logs data to SD card
4. Controls fan based on temperature

---

### 🧾 Example Code

```cpp
#include <Wire.h>
#include <Adafruit_SHT31.h>

Adafruit_SHT31 sht31 = Adafruit_SHT31();

#define FAN_PIN 33

float FAN_ON_TEMP = 22.5;
float FAN_OFF_TEMP = 21.5;

void setup() {
  Serial.begin(115200);
  sht31.begin(0x44);
  pinMode(FAN_PIN, OUTPUT);
}

void loop() {
  float temp = sht31.readTemperature();
  float hum = sht31.readHumidity();

  if (temp > FAN_ON_TEMP) {
    digitalWrite(FAN_PIN, HIGH);
  } else if (temp < FAN_OFF_TEMP) {
    digitalWrite(FAN_PIN, LOW);
  }

  Serial.print("Temp: "); Serial.println(temp);
  Serial.print("Humidity: "); Serial.println(hum);

  delay(2000);
}
