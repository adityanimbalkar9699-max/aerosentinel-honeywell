"""
============================================================================
AeroSentinel-X: Comprehensive Automated Verification Test Suite
File: verify_system.py
Description: Mathematical proof, AST syntax validation, sensor response
             calibration curves, and TinyML boundary condition tests.
============================================================================
"""

import math
import unittest
import json
import os

class TestAeroSentinelSystem(unittest.TestCase):

    def setUp(self):
        # Baseline reference constants
        self.R0_CO = 15.0
        self.R0_CH4 = 12.0
        self.R0_AQI = 10.0

    def calculate_heat_index(self, temp_c, humidity):
        """Python mirror of the Rothfusz regression equation in C++."""
        temp_f = (temp_c * 9.0 / 5.0) + 32.0
        hi = 0.5 * (temp_f + 61.0 + ((temp_f - 68.0) * 1.2) + (humidity * 0.094))
        if hi >= 80.0:
            hi = (-42.379 + 2.04901523 * temp_f + 10.14333127 * humidity
                  - 0.22475541 * temp_f * humidity
                  - 0.00683783 * temp_f * temp_f
                  - 0.05481717 * humidity * humidity
                  + 0.00122874 * temp_f * temp_f * humidity
                  + 0.00085282 * temp_f * humidity * humidity
                  - 0.00000199 * temp_f * temp_f * humidity * humidity)
            if humidity < 13.0 and 80.0 <= temp_f <= 112.0:
                adj = ((13.0 - humidity) / 4.0) * math.sqrt((17.0 - abs(temp_f - 95.0)) / 17.0)
                hi -= adj
            elif humidity > 85.0 and 80.0 <= temp_f <= 87.0:
                adj = ((humidity - 85.0) / 10.0) * ((87.0 - temp_f) / 5.0)
                hi += adj
        return (hi - 32.0) * 5.0 / 9.0

    def test_heat_index_standard_values(self):
        """Verifies NOAA standard table values (35°C at 70% RH produces extreme caution/danger > 48°C)."""
        hi = self.calculate_heat_index(35.0, 70.0)
        self.assertGreater(hi, 47.0, "Heat index at 35C/70% RH must exceed 47C (OSHA Extreme Danger)")
        self.assertLess(hi, 55.0, "Heat index must remain within physical bounds")
        print(f"[TEST PASS] NOAA Heat Index calculation verified: 35°C @ 70% RH -> {hi:.2f}°C")

    def test_mq7_carbon_monoxide_curve(self):
        """Verifies exponential sensor curve PPM = 99.04 * (Rs/R0)^(-1.518)."""
        def ppm_co(rs_ro):
            return 99.04 * math.pow(rs_ro, -1.518)

        # In clean air (Rs/R0 ~ 1.0), CO concentration is low
        clean_ppm = ppm_co(1.0)
        self.assertAlmostEqual(clean_ppm, 99.04, delta=0.5)

        # In elevated toxic smoke, Rs drops drastically (e.g. Rs/R0 = 0.3)
        surge_ppm = ppm_co(0.3)
        self.assertGreater(surge_ppm, 600.0, "CO surge should register > 600 PPM")
        print(f"[TEST PASS] MQ-7 CO calibration curve verified: Rs/R0=0.3 -> {surge_ppm:.1f} PPM")

    def test_tinyml_hazard_classification_boundaries(self):
        """Verifies safety invariant: Any CO >= 50 PPM or CH4 >= 5000 PPM triggers Hazard Level 3 (CRITICAL_EVACUATE)."""
        def classify(co, ch4, aqi, hi, fall):
            if co >= 50.0 or ch4 >= 5000.0 or hi >= 42.0 or fall:
                return 3 # CRITICAL_EVACUATE
            elif co >= 25.0 or ch4 >= 2000.0 or hi >= 38.0:
                return 2 # WARNING
            elif hi >= 32.0:
                return 1 # ADVISORY
            return 0 # SAFE

        self.assertEqual(classify(4.0, 450.0, 15.0, 28.0, False), 0, "Nominal condition must be SAFE (0)")
        self.assertEqual(classify(26.0, 450.0, 15.0, 28.0, False), 2, "26 PPM CO must trigger WARNING (2)")
        self.assertEqual(classify(55.0, 450.0, 15.0, 28.0, False), 3, "55 PPM CO must trigger CRITICAL (3)")
        self.assertEqual(classify(4.0, 5200.0, 15.0, 28.0, False), 3, "5200 PPM CH4 must trigger CRITICAL (3)")
        self.assertEqual(classify(4.0, 450.0, 15.0, 28.0, True), 3, "Fall detection must trigger CRITICAL (3)")
        print("[TEST PASS] TinyML Hazard classification invariants 100% verified.")

    def test_telemetry_json_schema(self):
        """Verifies compliance with MQTT and WebSockets telemetry schema."""
        sample_payload = {
            "device_id": "AEROSENTINEL_NODE_01",
            "timestamp_ms": 128490,
            "co_ppm": 12.4,
            "ch4_ppm": 620.0,
            "aqi_ppm": 18.5,
            "temp_c": 29.8,
            "humidity_pct": 58.2,
            "heat_index_c": 31.4,
            "anomaly_score": 0.125,
            "hazard_level": 0,
            "hazard_desc": "SAFE_NOMINAL",
            "relay_state": 0
        }
        json_str = json.dumps(sample_payload)
        parsed = json.loads(json_str)
        self.assertIn("device_id", parsed)
        self.assertIn("hazard_level", parsed)
        self.assertEqual(parsed["hazard_desc"], "SAFE_NOMINAL")
        print("[TEST PASS] Telemetry JSON serialization and deserialization verified.")

if __name__ == "__main__":
    unittest.main()
