\
/**
 * Water Level Controller (Arduino)
 * 
 * Controls a pump based on tank water level measured by an ultrasonic sensor.
 * - Turns pump ON when level is below ON_THRESHOLD_PERCENT
 * - Turns pump OFF when level is above OFF_THRESHOLD_PERCENT
 * 
 * Hardware: Arduino Uno/Nano, HC-SR04 (or JSN-SR04T), 5V relay module
 * License: MIT
 */

// ---------------- Configuration ----------------
const byte PIN_TRIG = 9;         // HC-SR04 TRIG
const byte PIN_ECHO = 8;         // HC-SR04 ECHO
const byte PIN_RELAY = 7;        // Relay IN
const byte PIN_BUZZER = 6;       // Optional; set to 255 to disable

const bool RELAY_ACTIVE_STATE = LOW; // Set to LOW for active-low relay modules, HIGH otherwise

const float TANK_DEPTH_CM = 150.0;   // Distance from sensor to tank bottom when EMPTY (cm)

const uint8_t ON_THRESHOLD_PERCENT  = 30; // % full at/below which pump turns ON
const uint8_t OFF_THRESHOLD_PERCENT = 90; // % full at/above which pump turns OFF

const uint8_t MEDIAN_SAMPLES = 7;     // Number of distance samples for median filter (odd number)
const uint16_t MEASUREMENT_INTERVAL_MS = 1000; // Time between measurements

// Speed of sound (approx) in cm/us at ~20-25°C
const float SOUND_SPEED_CM_PER_US = 0.0343;

// Safety/timeouts
const unsigned long ECHO_TIMEOUT_US = 30000UL; // ~5m max (one-way)
const uint16_t STALE_READING_LIMIT = 10;       // after N failed reads, turn pump OFF as failsafe
// ------------------------------------------------

unsigned long lastMeasureMs = 0;
bool pumpOn = false;
uint16_t staleReads = 0;

float distances[15]; // up to 15 samples if needed

float measureDistanceCm() {
  // Trigger the ultrasonic sensor and measure echo time
  digitalWrite(PIN_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);

  unsigned long duration = pulseIn(PIN_ECHO, HIGH, ECHO_TIMEOUT_US);
  if (duration == 0) {
    return NAN; // timeout/failure
  }
  // distance = (duration_us * speed_of_sound_cm/us) / 2 (round trip)
  float distance = (duration * SOUND_SPEED_CM_PER_US) / 2.0;
  return distance;
}

// Simple insertion sort for small arrays
void sortArray(float* arr, uint8_t n) {
  for (uint8_t i = 1; i < n; ++i) {
    float key = arr[i];
    int8_t j = i - 1;
    while (j >= 0 && arr[j] > key) {
      arr[j + 1] = arr[j];
      j--;
    }
    arr[j + 1] = key;
  }
}

float medianDistanceCm(uint8_t samples) {
  uint8_t count = 0;
  for (uint8_t i = 0; i < samples; ++i) {
    float d = measureDistanceCm();
    if (!isnan(d) && d > 1 && d < 1000) {
      distances[count++] = d;
    }
    delay(30);
  }
  if (count == 0) return NAN;
  sortArray(distances, count);
  return distances[count / 2];
}

uint8_t clampPercent(int p) {
  if (p < 0) return 0;
  if (p > 100) return 100;
  return (uint8_t)p;
}

void setRelay(bool on) {
  if (on) {
    digitalWrite(PIN_RELAY, RELAY_ACTIVE_STATE);
  } else {
    digitalWrite(PIN_RELAY, !RELAY_ACTIVE_STATE);
  }
  pumpOn = on;
}

void beep(uint8_t times = 1, uint16_t onMs = 80, uint16_t offMs = 80) {
  if (PIN_BUZZER == 255) return;
  for (uint8_t i = 0; i < times; i++) {
    digitalWrite(PIN_BUZZER, HIGH);
    delay(onMs);
    digitalWrite(PIN_BUZZER, LOW);
    delay(offMs);
  }
}

void setup() {
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
  pinMode(PIN_RELAY, OUTPUT);
  if (PIN_BUZZER != 255) pinMode(PIN_BUZZER, OUTPUT);

  // Safe default
  setRelay(false);

  Serial.begin(115200);
  delay(200);
  Serial.println(F("Water Level Controller - starting..."));
  Serial.print(F("Tank depth (cm): ")); Serial.println(TANK_DEPTH_CM);
  Serial.print(F("Thresholds ON/OFF (%): ")); Serial.print(ON_THRESHOLD_PERCENT); Serial.print("/"); Serial.println(OFF_THRESHOLD_PERCENT);

  beep(2, 60, 60);
}

void loop() {
  unsigned long now = millis();
  if (now - lastMeasureMs >= MEASUREMENT_INTERVAL_MS) {
    lastMeasureMs = now;

    float d = medianDistanceCm(MEDIAN_SAMPLES);
    if (isnan(d) || d <= 0 || d > 1000) {
      staleReads++;
      Serial.println(F("[WARN] Bad/timeout reading."));
      if (staleReads >= STALE_READING_LIMIT) {
        // Fail-safe: stop the pump
        if (pumpOn) {
          Serial.println(F("[FAILSAFE] Too many bad reads. Pump OFF."));
          setRelay(false);
          beep(3, 40, 40);
        }
        staleReads = 0;
      }
      return;
    }

    staleReads = 0;

    // Convert distance to % full.
    // distance_to_surface = d
    // water_height = TANK_DEPTH_CM - d
    // percent = (water_height / TANK_DEPTH_CM) * 100
    float waterHeight = TANK_DEPTH_CM - d;
    int percentFull = (int)round((waterHeight / TANK_DEPTH_CM) * 100.0);
    uint8_t pct = clampPercent(percentFull);

    // Control logic with hysteresis
    if (!pumpOn && pct <= ON_THRESHOLD_PERCENT) {
      setRelay(true);
      Serial.print(F("[CTRL] Pump ON @ ")); Serial.print(pct); Serial.println(F("%"));
      beep(1, 120, 0);
    } else if (pumpOn && pct >= OFF_THRESHOLD_PERCENT) {
      setRelay(false);
      Serial.print(F("[CTRL] Pump OFF @ ")); Serial.print(pct); Serial.println(F("%"));
      beep(2, 60, 60);
    }

    // Telemetry
    Serial.print(F("Distance(cm): ")); Serial.print(d, 1);
    Serial.print(F(" | Level(%): ")); Serial.print(pct);
    Serial.print(F(" | Pump: ")); Serial.println(pumpOn ? F("ON") : F("OFF"));
  }
}
