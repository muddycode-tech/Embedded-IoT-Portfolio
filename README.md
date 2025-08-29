# Water Level Controller (Arduino)

A simple, reliable **automatic water tank level controller** using an Arduino (Uno/Nano), an HC-SR04 ultrasonic sensor (mounted on the tank lid), and a relay to switch a pump. The controller turns the pump **ON** when the level drops below a low threshold and **OFF** when it reaches the high threshold (hysteresis to avoid rapid toggling).

> ✨ Works offline, requires no cloud, and uses median filtering to handle sensor noise.

---

## Features

- Percent-based water level using an ultrasonic sensor
- **Hysteresis control** (e.g., ON below 30%, OFF above 90%)
- Median of N readings + sanity checks to reduce noise
- Configurable pins and tank height
- Serial debug output
- Safe default states on boot

---

## Hardware

- **Arduino** Uno/Nano (5V)
- **Ultrasonic sensor** HC‑SR04 (or JSN-SR04T*)
- **Relay module** (5V, opto-isolated preferred) to switch pump contactor/SSR
- Optional: **Buzzer/LED** for low/high alert

> *For outdoor tanks or misty environments, consider a JSN‑SR04T (waterproof) and ensure proper mounting to avoid false echoes.*

### Pin Mapping (default)
| Component        | Arduino Pin |
|------------------|-------------|
| HC‑SR04 TRIG     | D9          |
| HC‑SR04 ECHO     | D8          |
| Relay IN         | D7          |
| Buzzer (optional)| D6          |

> Change these in `src/water_level_controller.ino` if needed.

---

## Wiring

1. Mount the ultrasonic sensor **facing down** from the tank lid.
2. Measure the **empty-tank distance** (lid to bottom) and set it as `TANK_DEPTH_CM` in the code.
3. Wire the relay **IN** pin to Arduino D7 (default). Use the relay to drive a **contactor/SSR** rated for your pump.
4. **Power**: Share GND between Arduino, sensor, and relay module. Provide adequate isolation for mains.

> ⚠️ **Safety**: Mains voltage is dangerous. If unsure, consult a qualified electrician. Use a contactor/SSR and proper fusing/earthing. Keep low‑voltage and mains wiring physically separated.

---

## How it works

- The controller measures the distance to the water surface, converts it to **% full** based on `TANK_DEPTH_CM`, then applies hysteresis:
  - Turn **ON** when `% full` ≤ `ON_THRESHOLD_PERCENT` (e.g., 30%)
  - Turn **OFF** when `% full` ≥ `OFF_THRESHOLD_PERCENT` (e.g., 90%)
- Relay logic supports **active‑LOW** or **active‑HIGH** modules via `RELAY_ACTIVE_STATE`.

---

## Quick Start

1. Open `src/water_level_controller.ino` in the Arduino IDE.
2. Select **Board**: Arduino Uno/Nano; select the proper COM port.
3. Edit the **Configuration** section:
   - `TANK_DEPTH_CM` → set to your tank empty depth (cm)
   - `ON_THRESHOLD_PERCENT` and `OFF_THRESHOLD_PERCENT`
   - Pins if different from defaults
4. Upload and open the **Serial Monitor** at `115200` baud to view live readings.

---

## Calibration Tips

- Take several manual distance measurements to confirm `TANK_DEPTH_CM`.
- If readings fluctuate, increase `MEDIAN_SAMPLES` or add a **PVC stilling tube**.
- Ensure the sensor sits flat; avoid mounting above inlet splashes.

---

## Folder Structure

```
water-level-controller/
├─ src/
│  └─ water_level_controller.ino
├─ docs/
│  └─ wiring-notes.md
├─ .gitignore
└─ README.md
```

---

👨‍💻 Author: [ADITYA RAJ](https://github.com/muddycode-tech)  
⭐ If you like this project, don’t forget to star the repo!
