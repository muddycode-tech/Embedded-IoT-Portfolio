# Wiring Notes

- **Ultrasonic (HC‑SR04)**: VCC=5V, GND=GND, TRIG=D9, ECHO=D8 (use voltage divider if your ECHO is 5V-only and board is 3.3V).
- **Relay**: VCC=5V, GND=GND, IN=D7. Prefer opto-isolated modules. Switch mains via a contactor/SSR rated above your pump's startup current.
- **Buzzer/LED** (optional): to D6 via resistor or module.

> For outdoor tanks consider JSN‑SR04T (waterproof). Update timing if sensor specs differ.
