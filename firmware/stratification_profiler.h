/**
 * ============================================================================
 * StrataShield-3D: Vertical Density-Stratification Profiler & Interlock Engine
 * File: stratification_profiler.h
 * Description: Vertical column spatial gradient mapping, chemocline boundary
 *              detection, and physical zero-entry authorization interlock.
 * ============================================================================
 */

#ifndef STRATIFICATION_PROFILER_H
#define STRATIFICATION_PROFILER_H

#include <Arduino.h>
#include <math.h>

#define MAX_DEPTH_SLICES 50 // 0.0m to 5.0m at 10cm steps

struct DepthSlice {
    float depth_m;
    float gas_co_ppm;
    float gas_ch4_ppm;
    float gas_h2s_ppm;
    float o2_pct;
    float temp_c;
    bool is_safe;
};

struct StratificationReport {
    float chemocline_depth_m;
    float max_gradient_ppm_per_m;
    bool zero_entry_locked;
    int total_breaches;
    String primary_hazard;
};

class StratificationEngine {
private:
    DepthSlice profile[MAX_DEPTH_SLICES];
    int current_slice_count;

public:
    StratificationEngine() : current_slice_count(0) {}

    void recordSlice(int index, float depth, float co, float ch4, float h2s, float o2, float temp) {
        if (index < 0 || index >= MAX_DEPTH_SLICES) return;
        profile[index].depth_m = depth;
        profile[index].gas_co_ppm = co;
        profile[index].gas_ch4_ppm = ch4;
        profile[index].gas_h2s_ppm = h2s;
        profile[index].o2_pct = o2;
        profile[index].temp_c = temp;

        // OSHA 29 CFR 1910.146 Confined Space Pass/Fail Criteria:
        // H2S < 10 ppm, CO < 25 ppm, CH4 < 2500 ppm (5% LEL), O2 between 19.5% and 23.5%
        bool slice_safe = (h2s < 10.0f) && (co < 25.0f) && (ch4 < 2500.0f) && (o2 >= 19.5f && o2 <= 23.5f);
        profile[index].is_safe = slice_safe;

        if (index >= current_slice_count) {
            current_slice_count = index + 1;
        }
    }

    StratificationReport analyzeVerticalColumn() {
        StratificationReport rep;
        rep.chemocline_depth_m = 0.0f;
        rep.max_gradient_ppm_per_m = 0.0f;
        rep.total_breaches = 0;
        rep.zero_entry_locked = false;
        rep.primary_hazard = "ALL_SLICES_CERTIFIED_SAFE";

        if (current_slice_count < 2) {
            rep.zero_entry_locked = true;
            rep.primary_hazard = "INSUFFICIENT_SCAN_DATA";
            return rep;
        }

        // 1. Detect breaches and spatial gradients along vertical descent
        for (int i = 0; i < current_slice_count; i++) {
            if (!profile[i].is_safe) {
                rep.total_breaches++;
                rep.zero_entry_locked = true; // Physical Interlock Trigger
                if (profile[i].gas_h2s_ppm >= 10.0f) {
                    rep.primary_hazard = "LETHAL_H2S_STRATIFICATION";
                } else if (profile[i].gas_ch4_ppm >= 2500.0f) {
                    rep.primary_hazard = "EXPLOSIVE_CH4_POCKET";
                } else if (profile[i].o2_pct < 19.5f) {
                    rep.primary_hazard = "HYPOXIC_O2_DEFICIENCY";
                }
            }

            // Compute discrete derivative d(H2S)/dz
            if (i > 0) {
                float dz = profile[i].depth_m - profile[i - 1].depth_m;
                if (dz > 0.001f) {
                    float gradient = fabsf(profile[i].gas_h2s_ppm - profile[i - 1].gas_h2s_ppm) / dz;
                    if (gradient > rep.max_gradient_ppm_per_m) {
                        rep.max_gradient_ppm_per_m = gradient;
                        rep.chemocline_depth_m = (profile[i].depth_m + profile[i - 1].depth_m) / 2.0f;
                    }
                }
            }
        }

        return rep;
    }
};

#endif // STRATIFICATION_PROFILER_H
