# AeroSentinel-X: Circuit Wiring & Pinout Guide

## 1. Master ESP32 Pin Allocation Table

```
                         ESP32 NodeMCU-32S (38-Pin)
                              +---------------+
                     3V3  --- | 3V3       GND | --- GND (Common Ground)
                      EN  --- | EN         23 | --- Status LED (Green)
          MQ-135 (AOUT)  --- | VP(36)     22 | --- I2C SCL (OLED & MPU6050)
            MQ-7 (AOUT)  --- | VN(39)      1 | --- TX0 (Serial Telemetry)
            MQ-4 (AOUT)  --- | 34          3 | --- RX0 (Serial Command)
                         --- | 35         21 | --- I2C SDA (OLED & MPU6050)
      DHT22 (DATA Signal)--- | 32         19 | --- Status LED (Red)
                         --- | 33         18 | --- Piezo Buzzer (PWM)
                         --- | 25          5 | --- Emergency Cutoff Relay
                         --- | 26         17 | --- Reserved (ESP-NOW Sync)
                         --- | 27         16 | --- Reserved
                         --- | 14          4 | --- DHT22 Data
                         --- | 12          0 | --- Boot (Pull-up)
                         --- | 13          2 | --- Onboard LED
                              +---------------+
```

## 2. Pin-to-Pin Module Interconnections

### A. Analog Gas Sensor Array (MQ-135, MQ-7, MQ-4)
- **VCC (All 3 Sensors):** Connect to **5V / VIN** rail (MQ heater coils require 5V DC for stoichiometric reaction).
- **GND (All 3 Sensors):** Connect to **Common GND**.
- **MQ-135 AOUT:** Connect to **GPIO 34** (ADC1_CH6).
- **MQ-7 AOUT:** Connect to **GPIO 35** (ADC1_CH7).
- **MQ-4 AOUT:** Connect to **GPIO 32** (ADC1_CH4).
*Note: Using ADC1 channels exclusively ensures analog conversions remain functional while Wi-Fi / ESP-NOW radio is active.*

### B. I2C Bus Devices (SSD1306 0.96" OLED & MPU6050 6-Axis IMU)
- **VCC:** Connect to **3.3V** rail.
- **GND:** Connect to **Common GND**.
- **SDA:** Connect to **GPIO 21** (Hardware I2C Data) with 4.7kΩ pull-up.
- **SCL:** Connect to **GPIO 22** (Hardware I2C Clock) with 4.7kΩ pull-up.
- **I2C Addresses:**
  - SSD1306 Display: `0x3C`
  - MPU6050 IMU: `0x68`

### C. Digital Temperature & Humidity Sensor (DHT22 / AM2302)
- **Pin 1 (VCC):** Connect to **3.3V**.
- **Pin 2 (DATA):** Connect to **GPIO 4** (with 10kΩ pull-up resistor to 3.3V).
- **Pin 3 (NC):** Not connected.
- **Pin 4 (GND):** Connect to **Common GND**.

### D. Audio-Visual Alarm & Relay Actuator
- **Active Piezo Buzzer (+):** Connect to **GPIO 18**; (-) to **GND**.
- **Red Hazard LED (+):** Connect via 220Ω current-limiting resistor to **GPIO 19**; (-) to **GND**.
- **Green Status LED (+):** Connect via 220Ω current-limiting resistor to **GPIO 23**; (-) to **GND**.
- **Relay Module IN:** Connect to **GPIO 5** (Active HIGH triggers emergency power/valve interlock).
