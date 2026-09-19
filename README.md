# Smart Dynamic Sampling Rate – Adaptive Monitoring System

## 📌 Project Overview

The **Smart Dynamic Sampling Rate – Adaptive Monitoring System** is an IoT-based patient monitoring prototype developed using an **ESP32**, **FreeRTOS**, **MQTT**, **Adafruit IO**, and **Wokwi**.

The system dynamically adjusts the patient monitoring sampling interval based on the patient's condition.

In **Automatic (AUTO) mode**:

- Stable patient condition → sampling interval is **30 seconds**
- Abnormal patient condition → sampling interval is **5 seconds**

In **Manual (MANUAL) mode**, the monitoring interval can be remotely selected through an **Adafruit IO dashboard** from **5 to 60 seconds**.

> **Note:** This project is an educational IoT prototype. Patient parameters are simulated using potentiometers in Wokwi and are not obtained from medical-grade sensors.

---

## 🎯 Objectives

- Implement adaptive patient monitoring using ESP32.
- Dynamically change the sampling interval based on patient status.
- Provide automatic and manual sampling modes.
- Monitor simulated Heart Rate, SpO₂, and Body Temperature.
- Send monitoring data using MQTT.
- Display patient information locally using an OLED display.
- Provide remote monitoring through an Adafruit IO dashboard.
- Increase monitoring frequency during abnormal conditions.
- Reduce unnecessary sampling during stable conditions.

---

## ⚙️ System Features

### Patient Parameter Monitoring

| Parameter | Simulated Range | Normal Range |
|-----------|-----------------|--------------|
| Heart Rate | 40–140 BPM | 60–100 BPM |
| SpO₂ | 85–100 % | ≥ 95 % |
| Body Temperature | 34–40 °C | 36.0–37.5 °C |

Three potentiometers are used in Wokwi to simulate the patient parameters.

### Automatic Sampling

The system automatically selects the sampling interval based on patient condition:

| Patient Condition | Sampling Interval |
|--------------------|-------------------|
| Stable | 30 seconds |
| Abnormal | 5 seconds |

If any monitored parameter falls outside its defined normal range, the patient status becomes **ABNORMAL**.

### Manual Sampling

In MANUAL mode, the sampling interval can be selected remotely from:

**5–60 seconds**

### Automatic Mode Switching

The system supports:

- AUTO mode
- MANUAL mode

When switching from MANUAL to AUTO, the system immediately recalculates the sampling interval according to the current patient condition.

---

## 🧠 System Architecture

```text
                    ┌───────────────────────┐
                    │      Wokwi Inputs     │
                    │                       │
                    │  HR Potentiometer     │
                    │  SpO₂ Potentiometer   │
                    │  Temperature Pot      │
                    └───────────┬───────────┘
                                │
                                ↓
                    ┌───────────────────────┐
                    │         ESP32         │
                    │                       │
                    │ Parameter Processing  │
                    │ Condition Detection   │
                    │ Adaptive Sampling     │
                    │ AUTO / MANUAL Mode    │
                    └───────┬───────┬───────┘
                            │       │
                ┌───────────┘       └────────────┐
                ↓                                ↓
       ┌────────────────┐               ┌─────────────────┐
       │   OLED Display │               │   Wi-Fi / MQTT  │
       └────────────────┘               └────────┬────────┘
                                                 │
                                                 ↓
                                      ┌────────────────────┐
                                      │    Adafruit IO     │
                                      │                    │
                                      │ Heart Rate         │
                                      │ SpO₂               │
                                      │ Temperature        │
                                      │ Patient Status     │
                                      │ Sampling Interval  │
                                      │ Sampling Mode      │
                                      └────────────────────┘
```

---

## 🔌 Hardware Used

- ESP32
- SSD1306 OLED Display
- 3 × Potentiometers
- Wi-Fi
- Wokwi simulation environment

### OLED Connections

| OLED Pin | ESP32 Pin |
|----------|-----------|
| VCC | 3.3V |
| GND | GND |
| SDA | GPIO 21 |
| SCL | GPIO 22 |

### Simulated Patient Inputs

| Parameter | ESP32 GPIO |
|-----------|------------|
| Heart Rate | GPIO 34 |
| SpO₂ | GPIO 35 |
| Temperature | GPIO 32 |

---

## 💻 Software and Technologies

- ESP32
- Arduino IDE / Arduino CLI
- FreeRTOS
- MQTT
- Adafruit IO
- Wokwi
- C/C++
- SSD1306 OLED Library
- Wi-Fi
- Git
- GitHub

---

## 📡 MQTT Communication

### Published Feeds

The ESP32 publishes:

```text
heart-rate
spo2
body-temperature
patient-status
active-sampling
```

### Subscribed Feeds

The ESP32 receives:

```text
sampling-interval
sampling-mode
```

### Communication Flow

```text
ESP32
  │
  ├── Publish → Heart Rate
  ├── Publish → SpO₂
  ├── Publish → Body Temperature
  ├── Publish → Patient Status
  └── Publish → Active Sampling Interval
                 │
                 ↓
            Adafruit IO
                 │
                 ├── Sampling Interval
                 └── Sampling Mode
                       │
                       ↓
                     ESP32
```

---

## 📊 Adafruit IO Dashboard

The dashboard contains:

### Monitoring Widgets

- Heart Rate Gauge
- SpO₂ Gauge
- Body Temperature Gauge
- Patient Status Indicator
- Active Sampling Interval Display

### Control Widgets

- Sampling Interval Slider
- Sampling Mode Control

The dashboard provides remote monitoring and control of the adaptive sampling system.

---

## 🔄 Operating Modes

### AUTO Mode

In AUTO mode, the sampling interval is automatically determined by patient status.

**Stable:**

```text
Status = STABLE
Sampling = 30 seconds
```

**Abnormal:**

```text
Status = ABNORMAL
Sampling = 5 seconds
```

### MANUAL Mode

In MANUAL mode, the sampling interval is controlled through the dashboard.

```text
Mode = MANUAL
Sampling Interval = 5–60 seconds
```

Example:

```text
Mode = MANUAL
Sampling Interval = 15 seconds
```

---

## 🧪 Testing and Validation

The system was tested using different simulated patient conditions and sampling configurations.

| Test | Condition | Expected Result | Status |
|------|-----------|-----------------|--------|
| 1 | Normal Monitoring | Stable, 30 s in AUTO | PASS |
| 2 | Low SpO₂ | Abnormal, 5 s in AUTO | PASS |
| 3 | High Heart Rate | Abnormal, 5 s in AUTO | PASS |
| 4 | Low Heart Rate | Abnormal, 5 s in AUTO | PASS |
| 5 | High Temperature | Abnormal, 5 s in AUTO | PASS |
| 6 | Low Temperature | Abnormal, 5 s in AUTO | PASS |
| 7 | Manual 5 s | Sampling = 5 s | PASS |
| 8 | Manual 30 s | Sampling = 30 s | PASS |
| 9 | Manual 60 s | Sampling = 60 s | PASS |
| 10 | MANUAL → AUTO | Automatic interval restored | PASS |

---

## 🖥️ Example Monitoring Conditions

### Stable Condition

```text
Heart Rate       : 87 BPM
SpO₂             : 96 %
Temperature      : 36.7 °C
Patient Status   : STABLE
Mode             : AUTO
Sampling         : 30 seconds
```

### Abnormal Condition

```text
Heart Rate       : 59 BPM
SpO₂             : 96 %
Temperature      : 36.7 °C
Patient Status   : ABNORMAL
Mode             : AUTO
Sampling         : 5 seconds
```

### Manual Monitoring

```text
Mode              : MANUAL
Sampling Interval : 15 seconds
```

---

## 🧩 Project Structure

```text
Task_5/
│
├── Task_5.ino
├── diagram.json
├── wokwi.toml
├── README.md
├── Task_5_Smart_Dynamic_Sampling_Rate_Report.pdf
│
└── build/
    ├── Task_5.ino.bin
    └── Task_5.ino.elf
```

> The `build/` directory contains generated compilation files and does not need to be committed if it is excluded using `.gitignore`.

---

## ▶️ Running the Project in Wokwi

### 1. Enter the project directory

```bash
cd ~/Task_5
```

### 2. Compile the ESP32 project

```bash
arduino-cli compile --fqbn esp32:esp32:esp32 --output-dir build .
```

### 3. Wokwi Configuration

The project uses:

```toml
[wokwi]
version = 1
firmware = 'build/Task_5.ino.bin'
elf = 'build/Task_5.ino.elf'
```

### 4. Wokwi Wi-Fi

The ESP32 connects to:

```text
SSID: Wokwi-GUEST
Password: empty
```

---

## 🔐 Security

Sensitive credentials must **not** be uploaded to GitHub.

Use placeholders in the source code:

```cpp
#define AIO_USERNAME "YOUR_USERNAME"
#define AIO_KEY      "YOUR_AIO_KEY"
```

Do not commit an actual **Adafruit IO key** to the repository.

If an API key or MQTT credential is accidentally exposed, it should be regenerated before publishing the project.

---

## 📈 Results

The implemented system successfully demonstrated:

- Patient parameter simulation.
- Abnormal condition detection.
- Automatic sampling-rate adaptation.
- Stable-condition sampling at 30 seconds.
- Abnormal-condition sampling at 5 seconds.
- Manual sampling control from 5–60 seconds.
- MQTT-based communication.
- Remote dashboard monitoring.
- Remote sampling-rate configuration.
- Automatic recovery from abnormal to stable monitoring.
- Successful MANUAL → AUTO mode transition.
- OLED-based local monitoring.

---

## 🚀 Future Improvements

The prototype can be extended with:

- Actual medical-grade sensors.
- Electrocardiogram (ECG) monitoring.
- Pulse oximeter sensor.
- Real body-temperature sensor.
- Battery-powered operation.
- Low-power sleep modes.
- Data logging.
- Historical patient data visualization.
- Additional safety and alarm mechanisms.
- Physical hardware implementation instead of Wokwi simulation.

---

## 📄 Project Report

A detailed project report is included:

```text
Task_5_Smart_Dynamic_Sampling_Rate_Report.pdf
```

The report contains:

- Project objective
- System overview
- Hardware and software
- Adaptive sampling logic
- Dashboard implementation
- Testing
- Results
- Screenshots
- Conclusion
- Security considerations

---

## ⚠️ Disclaimer

This project is an **educational and experimental prototype** developed for internship and embedded/IoT system development.

The patient parameters are simulated using potentiometers in the Wokwi environment. The system is **not a medical device** and should not be used for actual diagnosis, treatment, or clinical monitoring.

---

## 👨‍💻 Project Information

| Category | Details |
|----------|---------|
| Project | Smart Dynamic Sampling Rate – Adaptive Monitoring System |
| Platform | ESP32 |
| Simulation | Wokwi |
| Communication | MQTT |
| Cloud Dashboard | Adafruit IO |
| Framework | FreeRTOS |
| Programming Language | C/C++ |
| Application Area | IoT-Based Patient Monitoring |

---

## ⭐ Key Concept

> **Increase monitoring frequency when abnormal conditions are detected and reduce monitoring frequency when the patient remains stable.**

This project demonstrates an adaptive monitoring mechanism using ESP32, MQTT, FreeRTOS, and a cloud-based IoT dashboard.
