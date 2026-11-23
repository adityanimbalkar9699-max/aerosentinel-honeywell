"""
============================================================================
AeroSentinel-X: Edge-AI Industrial PPE Compliance & Safety Sentinel
File: ppe_compliance_monitor.py
Description: Edge computer vision engine for industrial worker PPE validation
             (Hard Hat, High-Vis Vest, Respiratory Mask) and automated audit logging.
============================================================================
"""

import time
import json
import os
import math

class PPEComplianceEngine:
    def __init__(self, confidence_threshold=0.75):
        self.conf_threshold = confidence_threshold
        self.classes = ["hardhat", "safety_vest", "protective_mask", "gloves", "ear_protection"]
        self.incident_log = []

    def evaluate_worker_state(self, detections, worker_id="W-4092"):
        """
        Evaluates a frame detection vector against OSHA 1910 PPE Mandate.
        Mandatory: hardhat, safety_vest.
        """
        has_hardhat = any(d['class'] == 'hardhat' and d['confidence'] >= self.conf_threshold for d in detections)
        has_vest = any(d['class'] == 'safety_vest' and d['confidence'] >= self.conf_threshold for d in detections)
        has_mask = any(d['class'] == 'protective_mask' and d['confidence'] >= self.conf_threshold for d in detections)

        is_compliant = has_hardhat and has_vest
        missing_items = []
        if not has_hardhat: missing_items.append("Hard Hat")
        if not has_vest: missing_items.append("High-Visibility Vest")

        result = {
            "timestamp": time.strftime("%Y-%m-%dT%H:%M:%SZ", time.gmtime()),
            "worker_id": worker_id,
            "compliant": is_compliant,
            "detected_items": [d['class'] for d in detections],
            "missing_items": missing_items,
            "risk_level": "CRITICAL" if not is_compliant else "NOMINAL"
        }

        if not is_compliant:
            self.incident_log.append(result)
        return result

    def run_benchmark_simulation(self):
        """Simulates 100 consecutive edge frames across various industrial shifts."""
        print("[PPE Engine] Running 100-frame synthetic benchmark...")
        t_start = time.time()
        
        test_scenarios = [
            [{"class": "hardhat", "confidence": 0.94}, {"class": "safety_vest", "confidence": 0.91}], # 100% compliant
            [{"class": "hardhat", "confidence": 0.88}], # Missing vest
            [{"class": "safety_vest", "confidence": 0.92}], # Missing hardhat
            [], # Total non-compliance
            [{"class": "hardhat", "confidence": 0.96}, {"class": "safety_vest", "confidence": 0.89}, {"class": "protective_mask", "confidence": 0.92}] # Fully equipped
        ]

        results = []
        for i in range(100):
            scenario = test_scenarios[i % len(test_scenarios)]
            res = self.evaluate_worker_state(scenario, worker_id=f"W-{1000 + i%10}")
            results.append(res)

        elapsed = time.time() - t_start
        fps = 100.0 / elapsed
        avg_latency_ms = (elapsed / 100.0) * 1000.0

        print(f"[PPE Engine] Benchmark Completed:")
        print(f"  Processed Frames: 100")
        print(f"  Total Duration:   {elapsed:.4f} s")
        print(f"  Throughput:       {fps:.1f} FPS")
        print(f"  Average Latency:  {avg_latency_ms:.2f} ms")
        print(f"  Total Incidents:  {len(self.incident_log)} non-compliant events logged.")
        return {
            "fps": fps,
            "latency_ms": avg_latency_ms,
            "incident_count": len(self.incident_log)
        }

if __name__ == "__main__":
    engine = PPEComplianceEngine()
    stats = engine.run_benchmark_simulation()
    
    # Save benchmark audit report
    out_path = r"S:\Antigravity\projects\aerosentinel-honeywell\edge_ai_vision\benchmark_audit.json"
    with open(out_path, "w", encoding="utf-8") as f:
        json.dump(stats, f, indent=2)
    print(f"[PPE Engine] Audit results exported to: {out_path}")
