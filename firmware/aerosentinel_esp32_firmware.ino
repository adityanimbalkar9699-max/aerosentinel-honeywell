/**
 * ============================================================================
 * AeroSentinel-X: Edge-AI Industrial Safety & Multi-Hazard Sensor Fusion Beacon
 * Honeywell Technologies - Future Skills Edge Program Flagship Prototype
 * File: aerosentinel_esp32_firmware.ino
 * Target: ESP32 Dual-Core (Xtensa LX6 @ 240 MHz)
 * ============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include "hardware_config.h"
#include "tinyml_anomaly_detector.h"

// --- Global System Objects ---
SensorTelemetry currentData;
TinyMLEdgeDetector edgeAI;

// FreeRTOS Task Handles
TaskHandle_t TaskSensorCore0;
TaskHandle_t TaskCommsCore1;

// Simulated Sensor Calibration Factors (R0 in clean air)
float R0_MQ135 = 10.0;
float R0_MQ7   = 15.0;
float R0_MQ4   = 12.0;

// Fall Detection State Tracking
unsigned long freefallStartTime = 0;
bool potentialFallFlag = false;

// Convert 12-bit ADC reading to Sensor Resistance (Rs)
float calculateRs(int rawADC) {
    if (rawADC <= 0) return 99999.0f;
    float vOut = (rawADC / ADC_RESOLUTION) * ADC_VOLTAGE_REF;
    return RL_LOAD_RESISTOR_KOHM * (ADC_VOLTAGE_REF - vOut) / vOut;
}

// Convert Rs/R0 ratio to PPM using sensor exponential power law
float getPPM_MQ7(float rs_ro) {
    // Curve for CO: PPM = 99.04 * (Rs/R0)^(-1.518)
    if (rs_ro <= 0.01f) return 1000.0f;
    return 99.04f * powf(rs_ro, -1.518f);
}

float getPPM_MQ4(float rs_ro) {
    // Curve for Methane (CH4): PPM = 1012.7 * (Rs/R0)^(-2.78)
    if (rs_ro <= 0.01f) return 10000.0f;
    return 1012.7f * powf(rs_ro, -2.78f);
}

float getPPM_MQ135(float rs_ro) {
    // Curve for Ammonia/Toxic AQI: PPM = 102.2 * (Rs/R0)^(-2.473)
    if (rs_ro <= 0.01f) return 500.0f;
    return 102.2f * powf(rs_ro, -2.473f);
}

// ============================================================================
// CORE 0 TASK: High-Frequency Sensor Acquisition & TinyML Edge Anomaly Engine
// ============================================================================
void SensorAcquisitionCore0(void *pvParameters) {
    Serial.println("[Core 0] Sensor Acquisition & Edge-AI Engine initialized.");

    for (;;) {
        // 1. Oversampled Analog Reads (16 samples per sensor for noise rejection)
        long sum_mq135 = 0;
        long sum_mq7 = 0;
        long sum_mq4 = 0;
        for (int i = 0; i < 16; i++) {
            sum_mq135 += analogRead(PIN_MQ135_ADC);
            sum_mq7 += analogRead(PIN_MQ7_ADC);
            sum_mq4 += analogRead(PIN_MQ4_ADC);
            delayMicroseconds(200);
        }

        float rs_135 = calculateRs(sum_mq135 / 16);
        float rs_7   = calculateRs(sum_mq7 / 16);
        float rs_4   = calculateRs(sum_mq4 / 16);

        // 2. Compute Calibrated PPM Concentrations
        currentData.gas_aqi_ppm = getPPM_MQ135(rs_135 / R0_MQ135);
        currentData.gas_co_ppm  = getPPM_MQ7(rs_7 / R0_MQ7);
        currentData.gas_ch4_ppm = getPPM_MQ4(rs_4 / R0_MQ4);

        // 3. Environmental Readings (Simulated or Real DHT22 / BME280)
        // Default nominal factory floor baseline
        currentData.temperature_c = 31.4f + (sin(millis() / 50000.0f) * 2.5f);
        currentData.humidity_pct  = 62.0f + (cos(millis() / 45000.0f) * 5.0f);
        currentData.heat_index_c  = edgeAI.computeHeatIndex(currentData.temperature_c, currentData.humidity_pct);

        // 4. MPU6050 Accelerometer Magnitude (Simulated default 1.0g resting gravity)
        currentData.accel_magnitude_g = 1.0f;
        currentData.fall_detected = false;

        // 5. Run TinyML Anomaly Detection (Inference Latency < 1.5ms)
        edgeAI.evaluateTelemetry(currentData, millis());

        // 6. Autonomous Hardware Interlock Control
        if (currentData.hazard_level == 3) {
            // CRITICAL: Activate Solenoid Cutoff Relay and High-Decibel Siren
            digitalWrite(PIN_RELAY_CUTOFF, HIGH);
            digitalWrite(PIN_LED_RED, HIGH);
            digitalWrite(PIN_LED_GREEN, LOW);
            tone(PIN_BUZZER_PWM, 2800); // 2.8 kHz piercing industrial warning
        } else if (currentData.hazard_level == 2) {
            // WARNING: Intermittent alarm tone and red LED pulse
            digitalWrite(PIN_RELAY_CUTOFF, LOW);
            digitalWrite(PIN_LED_RED, (millis() % 500 < 250) ? HIGH : LOW);
            digitalWrite(PIN_LED_GREEN, LOW);
            tone(PIN_BUZZER_PWM, 1400);
        } else {
            // SAFE / ADVISORY: Normal operation
            digitalWrite(PIN_RELAY_CUTOFF, LOW);
            digitalWrite(PIN_LED_RED, LOW);
            digitalWrite(PIN_LED_GREEN, HIGH);
            noTone(PIN_BUZZER_PWM);
        }

        vTaskDelay(pdMS_TO_TICKS(100)); // 10 Hz refresh rate
    }
}

// ============================================================================
// CORE 1 TASK: Telemetry Serialization, Network Dispatch & Visual Feedback
// ============================================================================
void TelemetryCore1(void *pvParameters) {
    Serial.println("[Core 1] Network Telemetry & JSON Dispatcher active.");

    for (;;) {
        // Construct Structured JSON Telemetry String
        String jsonPayload = "{";
        jsonPayload += "\"device_id\":\"AEROSENTINEL_NODE_01\",";
        jsonPayload += "\"timestamp_ms\":" + String(millis()) + ",";
        jsonPayload += "\"co_ppm\":" + String(currentData.gas_co_ppm, 2) + ",";
        jsonPayload += "\"ch4_ppm\":" + String(currentData.gas_ch4_ppm, 1) + ",";
        jsonPayload += "\"aqi_ppm\":" + String(currentData.gas_aqi_ppm, 2) + ",";
        jsonPayload += "\"temp_c\":" + String(currentData.temperature_c, 1) + ",";
        jsonPayload += "\"humidity_pct\":" + String(currentData.humidity_pct, 1) + ",";
        jsonPayload += "\"heat_index_c\":" + String(currentData.heat_index_c, 1) + ",";
        jsonPayload += "\"anomaly_score\":" + String(currentData.anomaly_score, 3) + ",";
        jsonPayload += "\"hazard_level\":" + String(currentData.hazard_level) + ",";
        jsonPayload += "\"hazard_desc\":\"" + 
            String((currentData.hazard_level == 3) ? "CRITICAL_EVACUATE" :
                   (currentData.hazard_level == 2) ? "WARNING_ELEVATED" :
                   (currentData.hazard_level == 1) ? "ADVISORY" : "SAFE_NOMINAL") + "\",";
        jsonPayload += "\"relay_state\":" + String((currentData.hazard_level == 3) ? 1 : 0);
        jsonPayload += "}";

        // Output to Serial Monitor for edge logging / serial bridge
        Serial.println(jsonPayload);

        vTaskDelay(pdMS_TO_TICKS(500)); // Telemetry broadcast every 500ms
    }
}

// ============================================================================
// ARDUINO SETUP & SYSTEM INITIALIZATION
// ============================================================================
void setup() {
    Serial.begin(115200);
    delay(500);

    Serial.println("=================================================");
    Serial.println(" AeroSentinel-X: Edge-AI Industrial Life-Safety ");
    Serial.println(" Honeywell Technologies Future Skills Edge 2026 ");
    Serial.println("=================================================");

    // Pin Modes Configuration
    pinMode(PIN_MQ135_ADC, INPUT);
    pinMode(PIN_MQ7_ADC, INPUT);
    pinMode(PIN_MQ4_ADC, INPUT);
    pinMode(PIN_DHT_DATA, INPUT);
    
    pinMode(PIN_BUZZER_PWM, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_RELAY_CUTOFF, OUTPUT);

    // Initial safe states
    digitalWrite(PIN_RELAY_CUTOFF, LOW);
    digitalWrite(PIN_LED_RED, LOW);
    digitalWrite(PIN_LED_GREEN, HIGH);
    noTone(PIN_BUZZER_PWM);

    // Initialize FreeRTOS Tasks on Dedicated Hardware Cores
    xTaskCreatePinnedToCore(
        SensorAcquisitionCore0,
        "SensorCore0",
        8192,
        NULL,
        2,              // High Priority
        &TaskSensorCore0,
        0               // Pinned to Core 0
    );

    xTaskCreatePinnedToCore(
        TelemetryCore1,
        "CommsCore1",
        8192,
        NULL,
        1,              // Standard Priority
        &TaskCommsCore1,
        1               // Pinned to Core 1
    );

    Serial.println("[System] FreeRTOS Multi-Core Tasks Pinned & Running.");
}

void loop() {
    // Empty: Execution handled autonomously by FreeRTOS tasks on Cores 0 & 1
    vTaskDelay(pdMS_TO_TICKS(1000));
}
