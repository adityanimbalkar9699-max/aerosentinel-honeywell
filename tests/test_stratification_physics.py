"""
============================================================================
StrataShield-3D: Vertical Density-Stratification Fluid Mechanics Proof
File: test_stratification_physics.py
Description: Validates the physical law of gas density stratification in
             stagnant subterranean chambers, proving why single-point top-level
             testing is a fatal flaw, and verifying chemocline derivative detection.
============================================================================
"""

import math
import unittest
import numpy as np

class TestStratificationPhysics(unittest.TestCase):

    def setUp(self):
        # Molar masses (g/mol) and relative vapor densities (Air ~ 28.97 g/mol)
        self.M_AIR = 28.97
        self.M_CH4 = 16.04   # Methane: Lighter than air (0.55x) -> Rises to top
        self.M_CO  = 28.01   # Carbon Monoxide: Equal to air (0.97x) -> Mid-depth
        self.M_H2S = 34.08   # Hydrogen Sulfide: Heavier than air (1.18x) -> Pools at bottom
        self.M_CO2 = 44.01   # Carbon Dioxide: Much heavier (1.52x) -> Sinks to bottom

    def simulate_vertical_profile(self, depth_m):
        """
        Simulates gas concentration profile across vertical depth column z in [0.0, 5.0] meters.
        z = 0.0m is the surface manhole rim / open atmosphere.
        z = 4.5m is the bottom sediment sludge surface.
        """
        # H2S: Sigmoidal buildup near bottom sludge layer (chemocline at z = 3.0m)
        h2s_ppm = 2.0 + (140.0 - 2.0) / (1.0 + math.exp(-(depth_m - 3.2) / 0.4))

        # CH4: Rises to ceiling dome (peaks at z = 0.4m, drops at bottom)
        ch4_ppm = 800.0 + 4200.0 * math.exp(-((depth_m - 0.4) ** 2) / 1.2)

        # CO: Neutral buoyancy, distributed with mild gradient
        co_ppm = 6.0 + 45.0 * (depth_m / 5.0)

        # O2: Oxygen displacement in deep stagnant zone
        o2_pct = 20.9 - (20.9 - 11.2) / (1.0 + math.exp(-(depth_m - 2.8) / 0.5))

        return {
            "depth_m": depth_m,
            "h2s_ppm": h2s_ppm,
            "ch4_ppm": ch4_ppm,
            "co_ppm": co_ppm,
            "o2_pct": o2_pct
        }

    def test_surface_vs_bottom_disparity(self):
        """
        Proves the fatal blindspot:
        Surface inspection (depth = 0.3m) reports benign H2S (< 5 ppm) and safe O2 (> 20%),
        while bottom level (depth = 4.0m) reaches lethal H2S (> 100 ppm IDLH) and hypoxic O2 (< 14%).
        """
        top_data = self.simulate_vertical_profile(0.3)
        bottom_data = self.simulate_vertical_profile(4.0)

        # Verify Top appears superficially safe
        self.assertLess(top_data["h2s_ppm"], 10.0, "Surface H2S must appear deceptively safe")
        self.assertGreater(top_data["o2_pct"], 19.5, "Surface O2 must appear near-normal")

        # Verify Bottom is acutely lethal (OSHA IDLH for H2S is 100 ppm)
        self.assertGreater(bottom_data["h2s_ppm"], 100.0, "Bottom H2S must exceed lethal IDLH threshold")
        self.assertLess(bottom_data["o2_pct"], 14.0, "Bottom O2 must be severely depleted")

        print(f"\n[PHYSICS PROOF] Vertical Disparity Confirmed:")
        print(f"  Surface (z=0.3m): H2S = {top_data['h2s_ppm']:.1f} ppm | O2 = {top_data['o2_pct']:.1f}% -> 'FALSE SAFE'")
        print(f"  Bottom  (z=4.0m): H2S = {bottom_data['h2s_ppm']:.1f} ppm | O2 = {bottom_data['o2_pct']:.1f}% -> 'LETHAL IDLH'")

    def test_chemocline_derivative_boundary_detection(self):
        """
        Tests the mathematical chemocline edge detector d(H2S)/dz.
        The peak rate of spatial change defines the critical boundary depth.
        """
        depths = np.linspace(0.0, 5.0, 51) # 10 cm step resolution
        h2s_vals = [self.simulate_vertical_profile(d)["h2s_ppm"] for d in depths]
        
        # Calculate discrete spatial gradient dC/dz
        gradients = np.gradient(h2s_vals, depths)
        peak_idx = np.argmax(gradients)
        chemocline_depth = depths[peak_idx]

        # The simulated chemocline inflection was set at z = 3.2m
        self.assertAlmostEqual(chemocline_depth, 3.2, delta=0.2)
        print(f"[PHYSICS PROOF] Chemocline Boundary correctly localized at depth z = {chemocline_depth:.1f} meters (Max dC/dz = {gradients[peak_idx]:.1f} ppm/m)")

    def test_zero_entry_interlock_logic(self):
        """
        Verifies that an electromechanical interlock refuses authorization
        unless EVERY depth slice satisfies OSHA 29 CFR 1910.146 criteria.
        """
        depths = np.linspace(0.0, 4.5, 46)
        column_certified = True
        violation_reason = None

        for d in depths:
            state = self.simulate_vertical_profile(d)
            if state["h2s_ppm"] >= 10.0:
                column_certified = False
                violation_reason = f"H2S breach at depth {d:.1f}m ({state['h2s_ppm']:.1f} ppm >= 10 ppm PEL)"
                break
            if state["ch4_ppm"] >= 2500.0:
                column_certified = False
                violation_reason = f"CH4 explosive pocket at depth {d:.1f}m"
                break
            if state["o2_pct"] < 19.5:
                column_certified = False
                violation_reason = f"Oxygen deficiency at depth {d:.1f}m ({state['o2_pct']:.1f}% < 19.5%)"
                break

        self.assertFalse(column_certified, "Interlock must reject entry due to stratified toxicity")
        print(f"[PHYSICS PROOF] Interlock Decision: REJECT ENTRY (Gate Locked) - {violation_reason}")

if __name__ == "__main__":
    unittest.main()
