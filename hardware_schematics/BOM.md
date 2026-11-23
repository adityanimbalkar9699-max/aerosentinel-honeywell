# AeroSentinel-X: Hardware Bill of Materials (BOM) & Electrical Analysis
**Project:** AeroSentinel-X — Edge-AI Industrial Safety & Multi-Hazard Sensor Fusion System  
**Organization:** Honeywell Technologies – Future Skills Edge Program (Partner: BharatCares)  
**Target Unit Cost:** < ₹3,200 ($38.50 USD) | Commercial Honeywell Equivalent: ₹45,000+ ($540 USD)

---

## 1. Complete Component Breakdown

| Item # | Component Name | Model / Specification | Operating Voltage | Interface | Approx. Cost (INR) | Supplier / Source |
| :--- | :--- | :--- | :--- | :--- | :--- | :--- |
| **1** | **Microcontroller MCU** | ESP32-WROOM-32 (Dual-Core Xtensa @ 240MHz, 520KB SRAM) | 3.3V / 5V | SPI/I2C/UART/ADC | ₹420 | Robu.in / ElectronicsComp |
| **2** | **Toxic Air / NH3 Sensor** | MQ-135 Hazardous Gas Sensor (Ammonia, Benzene, Alcohol) | 5.0V (Heater) | Analog ADC (GPIO34) | ₹145 | Robu.in |
| **3** | **Carbon Monoxide Sensor** | MQ-7 Electrochemical Catalytic CO Sensor | 5.0V / 1.4V cycle | Analog ADC (GPIO35) | ₹165 | Robu.in |
| **4** | **Combustible Methane Sensor** | MQ-4 Natural Gas / Methane (CH4) Sensor | 5.0V | Analog ADC (GPIO32) | ₹155 | Robu.in |
| **5** | **Environmental Temp & Humidity** | DHT22 / AM2302 (Precision ±0.5°C, ±2% RH) | 3.3V – 5.0V | 1-Wire (GPIO4) | ₹260 | Robu.in |
| **6** | **6-DOF IMU (Fall / Man-Down)** | MPU-6050 (3-Axis Accelerometer + 3-Axis Gyroscope) | 3.3V | I2C (0x68) | ₹180 | Robu.in |
| **7** | **Visual Display** | 0.96" SSD1306 OLED (128x64 White/Blue) | 3.3V | I2C (0x3C) | ₹210 | Robu.in |
| **8** | **High-Decibel Acoustic Alarm** | Active Piezo Buzzer 85dB @ 10cm (Continuous/Pulsed) | 3.3V – 5.0V | PWM (GPIO18) | ₹25 | Robu.in |
| **9** | **Tri-Color LED Beacon** | 5mm High-Luminosity RGB LED (Common Cathode) | 3.3V | GPIO19 / GPIO23 | ₹15 | Local Electronics |
| **10** | **Emergency Cutoff Relay** | 5V 1-Channel Optocoupler Relay Module (10A 250VAC) | 5.0V | Digital (GPIO5) | ₹65 | Robu.in |
| **11** | **Power Supply & Battery** | 18650 Li-Ion Cell (3.7V 2600mAh) + TP4056 BMS + Step-Up Boost | 3.7V to 5V | Micro-USB / Type-C | ₹340 | Robu.in |
| **12** | **Enclosure & Hardware** | Industrial IP54 Polycarbonate Handheld Enclosure & PCB | N/A | Mechanical | ₹380 | Local Fabricator |
| **13** | **Passive Components** | 10kΩ resistors, decoupling capacitors, DuPont jumpers | N/A | Hardware | ₹80 | Lab Stock |
| **TOTAL** | **Complete Edge Hardware Unit** | **Turnkey Industrial Life-Safety Sentinel** | **5.0V DC** | **Multi-Bus** | **₹2,445** | **1/18th Enterprise Cost** |

---

## 2. Power Consumption & Battery Autonomy Calculation
- **ESP32 Active (Wi-Fi + BLE Beacon + Edge AI Core):** ~140 mA @ 3.3V
- **MQ Gas Sensor Heaters (3 units @ ~150 mA pulsed duty cycle):** ~280 mA @ 5V
- **OLED Display & Indicators:** ~20 mA @ 3.3V
- **Total Peak Current:** ~440 mA @ 5V (~2.2 Watts)
- **Sleep / Intermittent Sampling Current (1 Hz cycle):** ~85 mA
- **Operational Battery Life (2600mAh 18650 Cell):**
  $$\text{Battery Life} = \frac{2600\text{ mAh} \times 0.85\text{ (efficiency)}}{85\text{ mA}} \approx 26.0\text{ Hours}$$
  *Sufficient for full 3-shift 24-hour continuous industrial operation before recharging.*
