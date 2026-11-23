/**
 * ============================================================================
 * AeroSentinel-X: Edge-AI Industrial Multi-Hazard Safety Sentinel
 * Honeywell Technologies - Future Skills Edge Program Flagship Prototype
 * File: hardware_config.h
 * Description: Hardware pin definitions, sensor calibrations, and system constants
 * ============================================================================
 */

#ifndef HARDWARE_CONFIG_H
#define HARDWARE_CONFIG_H

// --- Hardware Pin Allocations (ESP32-WROOM-32) ---
#define PIN_MQ135_ADC       34  // Analog Input: Air Quality / Toxic Gas (NH3, Benzene)
#define PIN_MQ7_ADC         35  // Analog Input: Carbon Monoxide (CO)
#define PIN_MQ4_ADC         32  // Analog Input: Methane (CH4) / Flammable Gases
#define PIN_DHT_DATA        4   // Digital 1-Wire: DHT22 / AM2302 Temp & Humidity
#define PIN_I2C_SDA         21  // I2C Data: SSD1306 OLED & MPU6050 IMU
#define PIN_I2C_SCL         22  // I2C Clock: SSD1306 OLED & MPU6050 IMU

#define PIN_BUZZER_PWM      18  // Output: High-decibel Piezo Audio Alarm (PWM)
#define PIN_LED_RED         19  // Output: Red Hazard Indicator LED
#define PIN_LED_GREEN       23  // Output: Green Safe Status LED
#define PIN_RELAY_CUTOFF    5   // Output: Emergency Valve / Power Interlock Relay

// --- Sensor Calibration & Environmental Thresholds (OSHA & NIOSH Standards) ---
// Carbon Monoxide (CO): OSHA PEL = 50 ppm, TWA NIOSH = 35 ppm, IDLH = 1200 ppm
#define THRESHOLD_CO_WARNING_PPM    25.0f
#define THRESHOLD_CO_DANGER_PPM     50.0f

// Methane (CH4): Lower Explosive Limit (LEL) = 5% (50,000 ppm), 10% LEL = 5,000 ppm
#define THRESHOLD_CH4_WARNING_PPM   2000.0f
#define THRESHOLD_CH4_DANGER_PPM    5000.0f

// Toxic Air / Ammonia (NH3): OSHA PEL = 50 ppm, NIOSH REL = 25 ppm
#define THRESHOLD_AQI_WARNING_PPM   50.0f
#define THRESHOLD_AQI_DANGER_PPM    100.0f

// Heat Index (°C) Thresholds (NOAA / OSHA Occupational Heat Stress)
#define THRESHOLD_HEAT_CAUTION      32.0f  // Caution: Fatigue possible with prolonged exposure
#define THRESHOLD_HEAT_DANGER       39.0f  // Danger: Heat cramps, heat exhaustion, impending stroke

// MPU6050 Fall Detection Invariants (g = 9.81 m/s^2)
#define FALL_FREEFALL_THRESHOLD     0.4f   // Vector magnitude drops during free-fall
#define FALL_IMPACT_THRESHOLD       2.8f   // Vector magnitude spike upon ground impact
#define FALL_INACTIVITY_MS          4000   // Post-impact lack of motion window

// --- System Operational Parameters ---
#define ADC_RESOLUTION              4096.0f // 12-bit ADC on ESP32
#define ADC_VOLTAGE_REF             3.3f    // Reference voltage
#define RL_LOAD_RESISTOR_KOHM       10.0f   // Load resistor on breakout boards
#define MQ_CLEAN_AIR_RATIO          3.6f    // Baseline Ro ratio in clean atmospheric air

#endif // HARDWARE_CONFIG_H
