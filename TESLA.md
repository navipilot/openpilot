# Tesla Legacy Support for FrogPilot on comma 3

**Branch:** `tesla-xnor-c3-dev`
**Base:** FrogPilot-Development + [xnor-tech/openpilot](https://github.com/xnor-tech/openpilot) Tesla legacy support
**Hardware:** comma 3 with [xnor harness kit](https://github.com/xnor-tech/openpilot)
**Supported cars:** Tesla Model S and Model X (HW1, HW2, HW2.5, HW3)

---

## Overview

This branch adds Tesla Model S/X legacy support to FrogPilot running on a comma 3. It builds on [xnor-tech's](https://github.com/xnor-tech/openpilot) foundational work for Tesla legacy CAN integration and extends it with FrogPilot-specific features: cooperative steering, Always On Lateral (AOL), and Speed Limit Controller (SLC) integration.

The focus is on improving the quality of daily driving for pre-refresh Tesla Model S/X vehicles where the stock Autopilot hardware (HW2/HW2.5) is aging and limited.

---

## Changes from FrogPilot-Development

### 1. Tesla Legacy Platform Support
**Commit:** `1b5e449a01` + fixup commits (`9e351d56` through `9869a517`)

Integrates xnor-tech's Tesla legacy platform into the FrogPilot codebase:

- **CAN bus architecture:** Party bus (bus 0), vehicle bus (bus 1), autopilot party (bus 2), powertrain (bus 4), chassis (bus 5), autopilot powertrain (bus 6). HW1 cars remap powertrain to party bus.
- **Fingerprints:** `TESLA_MODEL_S_HW1`, `TESLA_MODEL_S_HW3` (also used by HW2/HW2.5 cars), `TESLA_MODEL_X_HW1`
- **Safety mode:** `tesla_legacy` with steer angle limits, EAC fault detection, and cooperative steering support
- **Steering:** Angle-based control via EPAS (Electric Power Assisted Steering) at 50 Hz
- **Radar:** Tesla Bosch radar integration via `radar_interface.py`
- **Legacy CAN protocol:** `teslacan_legacy.py` (TeslaCANRaven) handles the older CAN message format with counter-based steering commands

**Files:**
- `opendbc_repo/opendbc/car/tesla/` — carstate, carcontroller, interface, fingerprints, values, teslacan_legacy
- `opendbc_repo/opendbc/safety/modes/tesla_legacy.h` — panda safety mode
- `opendbc_repo/opendbc/car/torque_data/override.toml` — steering torque parameters
- `opendbc_repo/opendbc/dbc/tesla_raven_party.dbc` — CAN message definitions
- `panda/board/` — harness and CAN driver support

### 2. Always On Lateral (AOL) for Tesla Legacy
**Commit:** `8fe0ee9cc0`

Enables the `ALWAYS_ON_LATERAL` alternative experience flag in the `tesla_legacy` safety mode, allowing lateral steering control to remain active even when cruise is disengaged.

**File:** `opendbc_repo/opendbc/safety/modes/tesla_legacy.h`

### 3. AOL Persistence Fix
**Commit:** `5295aef2fc`

**Problem:** Tesla's autopilot ECU flickers `cruiseState.available` on local streets — the autopilot icon flashes available/unavailable during tight turns, low speeds, or when the ECU decides conditions aren't suitable. FrogPilot's generic AOL logic ties `always_on_lateral_allowed` directly to this signal every frame, so any flicker immediately kills AOL and lateral control stops.

**Fix:** Added a Tesla-specific brand check in `frogpilot_card.py` (following the same pattern as the existing Hyundai-specific logic). For Tesla, `always_on_lateral_allowed` is unconditionally `True` when the AOL setting is enabled, bypassing the unreliable `cruiseState.available` signal. All safety gating still applies through the `always_on_lateral_enabled` conditions (non-driving gears, lateral check, calibration, immediate disable alerts, brake/standstill).

**File:** `frogpilot/controls/frogpilot_card.py`

### 4. Cooperative Steering
**Commit:** `f083a535d6`

**Problem:** Tesla's EPAS enforces a hard override — when the driver applies steering torque above a threshold, the EPS can fault (`EAC_INHIBITED`) and disable steering entirely. On newer Teslas this is handled by checking `hands_on_level < 3`, but legacy cars need a smoother approach.

**Solution:** A cooperative steering state machine (`CoopSteering`) that:
1. **Detects driver input** when `|steeringTorque| > 0.5 Nm` or `hands_on_level >= 2`
2. **During override:** Commands the car's physical steering angle instead of openpilot's desired angle, preventing any fight between driver and system
3. **On release:** Waits 0.5 seconds (configurable `RESUME_DELAY`), then rate-limits back to openpilot's desired angle via the existing `apply_steer_angle_limits_vm` function
4. **When lat_active is False:** Resets immediately, no residual state

This allows the driver to smoothly take over steering (e.g., obstacle avoidance, lane positioning) without triggering faults, and openpilot seamlessly resumes control after the driver releases.

**Files:**
- `opendbc_repo/opendbc/car/tesla/coopsteering.py` — state machine
- `opendbc_repo/opendbc/car/tesla/carcontroller.py` — integration into steering command pipeline
- `opendbc_repo/opendbc/car/tesla/carstate.py` — `steeringDisengage` detection from EAC status

### 5. Speed Limit Controller (SLC) — Dashboard Source
**Commits:** `e98a2d5166`, `3747eeafaf`, `0ade779a5e`

**Problem:** The SLC "Dashboard" source was always showing N/A for HW2.5 cars fingerprinting as `TESLA_MODEL_S_HW3`. The `DAS_fusedSpeedLimit` signal in CAN message 921 (`AutopilotStatus`) was being read from bus 2 (autopilot party), but on HW2.5 cars this message is broadcast on bus 1 (vehicle bus).

**Investigation:** Used pycapnp to parse rlog CAN data directly from the comma device. Confirmed message 921 exists on bus 1, not bus 2, for this hardware configuration.

**Fix:**
- Added `AutopilotStatus` message definition to `tesla_raven_party.dbc` (commit `3747eeaf`)
- Added a `Bus.main` CAN parser on `CANBUS.vehicle` (bus 1) specifically for HW3-fingerprinted cars (commit `0ade779a`)
- Modified `update_legacy()` to read `AutopilotStatus` from `Bus.main` for HW3, `Bus.ap_party` for other legacy cars
- SLC uses `DAS_fusedSpeedLimit` (scale=5, range 1-150 kph/mph) from Tesla's own speed limit fusion, converting to m/s for the FrogPilot speed limit controller

**Also:** Made SLC authoritative over Tesla's PCM cruise speed (commit `e98a2d51`) so the speed limit controller can actively manage cruise set speed.

**Files:**
- `opendbc_repo/opendbc/car/tesla/carstate.py` — bus routing and signal reading
- `opendbc_repo/opendbc/dbc/tesla_raven_party.dbc` — CAN message definitions
- `frogpilot/controls/lib/frogpilot_vcruise.py` — SLC cruise speed authority

### 6. Jerk Ramp After Gas Override
**Source:** [dzid26/opendbc vtb branch](https://github.com/dzid26/opendbc/tree/vtb) — `teslacan.py`, `values.py`

**Problem:** When the driver presses gas while cruise is active and traveling above the set speed, then releases gas, the Tesla PCM transitions from OVERRIDE back to ENABLED. At that moment openpilot sends a sudden deceleration command with full `JERK_LIMIT_MAX` (4.9 m/s³), causing a jarring lurch.

**Fix (two parts):**
1. **set_speed:** During `cruise_override` (DI_cruiseState == OVERRIDE), keep `set_speed = V_CRUISE_MAX` instead of setting it to 0 on decel.
2. **Jerk ramp:** When cruise override ends, `DAS_jerkMax` resets to 0 and ramps up at `JERK_RATE_UP` (1.0 m/s³/s) over ~5 seconds — a soft start for longitudinal control after gas override.

**Files:**
- `opendbc_repo/opendbc/car/tesla/teslacan.py` — jerk ramp logic in `create_longitudinal_command`
- `opendbc_repo/opendbc/car/tesla/teslacan_legacy.py` — same fix for legacy cars
- `opendbc_repo/opendbc/car/tesla/carstate.py` — `cruise_override` state tracking
- `opendbc_repo/opendbc/car/tesla/carcontroller.py` — passes `cruise_override` to longitudinal command
- `opendbc_repo/opendbc/car/tesla/values.py` — `JERK_RATE_UP` constant

### 7. Cruise Speed Alignment and DBC Fix
**Source:** [dzid26/opendbc vtb branch](https://github.com/dzid26/opendbc/tree/vtb) — `carstate.py`, `tesla_model3_party.dbc`

**Problem:** Tesla's cluster display speed (`DI_uiSpeed`) is ~1% higher than `DI_vehicleSpeed`. Without compensating, openpilot's cruise target overshoots the displayed set speed on the Tesla dashboard.

**Fix:**
- Read `DI_uiSpeed` and `DI_uiSpeedUnits` to set `vEgoCluster`
- Store cruise set speed in `cruiseState.speedCluster`, divide by 1.01 for `cruiseState.speed`
- Fixed `DI_uiSpeedUnits` DBC bit position from 32 to 33 (genuine bug)

**Files:**
- `opendbc_repo/opendbc/car/tesla/carstate.py` — `vEgoCluster` and `speedCluster`
- `opendbc_repo/opendbc/dbc/tesla_model3_party.dbc` — bit position fix

### 8. invalidLkasSetting: Check Autopilot Engagement State
**Source:** [dzid26/opendbc vtb branch](https://github.com/dzid26/opendbc/tree/vtb) — `carstate.py`

Changed from checking `DAS_autosteerEnabled` (settings flag) to `DAS_autopilotState` from `DAS_status` (actual engagement state). States 0/1/2 are safe; anything else means Autopilot/FSD is actively steering.

**File:** `opendbc_repo/opendbc/car/tesla/carstate.py`

### 9. Broaden Stock Steering Control Detection
**Source:** [dzid26/opendbc vtb branch](https://github.com/dzid26/opendbc/tree/vtb) — `tesla.h`

Broadened from `steering_control_type == 2` (LKAS only) to `steering_control_type != 0` (any non-NONE). Catches LDA, ELDA, Autopark steering — any stock system controlling the wheel. Changed edge detection from `!controls_allowed` to `!is_lat_active()`.

**Files:**
- `opendbc_repo/opendbc/safety/modes/tesla.h` — `tesla_stock_lkas` → `tesla_stock_steering_control`
- `opendbc_repo/opendbc/safety/modes/tesla_legacy.h` — same for legacy

### 10. Autopark → Summon Rename
**Source:** [dzid26/opendbc vtb branch](https://github.com/dzid26/opendbc/tree/vtb) — `carstate.py`, `tesla.h`

`DI_autoparkState` is used by Summon (including Smart Summon), not Autopark. Renamed all internal variables for semantic correctness. No behavioral change.

**Files:**
- `opendbc_repo/opendbc/car/tesla/carstate.py` — `autopark` → `summon`
- `opendbc_repo/opendbc/safety/modes/tesla.h` — `tesla_autopark` → `tesla_summon`
- `opendbc_repo/opendbc/safety/tests/libsafety/safety.c`

---

## CAN Bus Architecture (Tesla Legacy)

| Bus | Name | Description |
|-----|------|-------------|
| 0 | Party | Main bus — steering commands, vehicle controls |
| 1 | Vehicle | Vehicle status, DI_state, AutopilotStatus (msg 921) |
| 2 | Autopilot Party | Autopilot ECU messages (on some configurations) |
| 4 | Powertrain | Drive inverter, motor status |
| 5 | Chassis | DI_state, DI_torque2, gear, cruise state |
| 6 | Autopilot Powertrain | Autopilot powertrain messages |

**Note:** HW1 cars remap powertrain (bus 4) to party (bus 0). HW2.5 cars fingerprinting as HW3 broadcast `AutopilotStatus` on bus 1 instead of bus 2.

---

## Known Behaviors

- **Low-speed cooperative steering:** At very low speeds (parking lots, tight turns), the driver is constantly steering, so cooperative steering remains in permanent override mode. This is expected — openpilot shouldn't fight the driver during parking maneuvers.
- **AOL at standstill:** The brake + speed condition can flicker at the exact standstill boundary (vEgo oscillating around 0 m/s). This causes brief AOL on/off cycling but is cosmetic — lateral control isn't meaningful at standstill.
- **HW2.5 fingerprinting:** HW2.5 cars may fingerprint as either `TESLA_MODEL_S_HW3` or `TESLA_MODEL_S_HW1` depending on firmware. The bus routing for `AutopilotStatus` adapts accordingly.

---

## Credits

- **[BogGyver](https://github.com/BogGyver)** — Pioneer of openpilot on Tesla, early Tesla integration work that paved the way for community support
- **[xnor-tech](https://github.com/xnor-tech/openpilot)** ([lukasloetkolben](https://github.com/lukasloetkolben)) — Tesla Model S/X legacy platform support, harness kit design, safety mode implementation, and dedicated ongoing work to support Tesla in upstream openpilot branches
- **[dzid26](https://github.com/dzid26)** ([opendbc vtb branch](https://github.com/dzid26/opendbc/tree/vtb)) — Jerk ramp after gas override, cruise speed alignment, DBC signal fix, invalidLkasSetting improvement, stock steering control broadening, and autopark→summon corrections. Based on sunnypilot's Tesla integration
- **[FrogAi/FrogPilot](https://github.com/FrogAi/FrogPilot)** — Base fork with AOL, SLC, CEM, and extensive customization framework
- **[comma.ai](https://github.com/commaai/openpilot)** — openpilot core
