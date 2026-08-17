/*
 * FloodGuard Edge-AI
 * Edge-based multi-sensor fuzzy intelligence for predictive flood early warning
 *
 * Phase 1 firmware skeleton derived from the previous flood-monitoring prototype.
 * This version intentionally separates acquisition, validation, fuzzy risk,
 * prediction and communications so the research implementation can be tested
 * and measured independently.
 *
 * Target: ESP32
 *
 * Planned sensors:
 *   JSN-SR04T waterproof ultrasonic -> water level
 *   Tipping-bucket rain gauge      -> rainfall accumulation/intensity
 *   Capacitive soil sensor         -> soil moisture
 *   BME280                         -> temperature/humidity/pressure
 *
 * Local warning must work without Wi-Fi/Firebase.
 */

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_BME280.h>

// ---------------- Pin map ----------------
#define WATER_TRIG 5
#define WATER_ECHO 18
#define RAIN_PIN 19
#define SOIL_PIN 34
#define BUZZER_PIN 25
#define LED_GREEN 26
#define LED_YELLOW 27
#define LED_RED 14

// OLED and BME280 share I2C: SDA=21, SCL=22
#define I2C_SDA 21
#define I2C_SCL 22

// ---------------- Sampling ----------------
constexpr uint32_t SAMPLE_INTERVAL_MS = 2000;
constexpr uint8_t LEVEL_WINDOW = 10;
constexpr float WARNING_LEVEL_CM = 80.0f;

Adafruit_BME280 bme;
bool bmeAvailable = false;

float levelHistory[LEVEL_WINDOW] = {0};
uint8_t historyIndex = 0;
uint8_t historyCount = 0;
float previousLevelCm = NAN;
uint32_t previousSampleMs = 0;

volatile uint32_t rainTips = 0;
uint32_t previousRainTips = 0;
uint32_t rainWindowStartMs = 0;

// Calibrate this constant for the selected tipping-bucket gauge.
// Example: 0.2794 mm/tip is common for some gauges, but MUST be verified
// against the actual sensor used in experiments.
constexpr float MM_PER_TIP = 0.2794f;

struct SensorConfidence {
  float water;
  float rain;
  float soil;
  float environment;
};

struct RiskResult {
  float fuzzyRisk;
  float finalRisk;
  String state;
};

void IRAM_ATTR rainTipISR() {
  rainTips++;
}

float median3(float a, float b, float c) {
  if (a > b) { float t = a; a = b; b = t; }
  if (b > c) { float t = b; b = c; c = t; }
  if (a > b) { float t = a; a = b; b = t; }
  return b;
}

float readRawWaterLevelCm() {
  digitalWrite(WATER_TRIG, LOW);
  delayMicroseconds(3);
  digitalWrite(WATER_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(WATER_TRIG, LOW);

  unsigned long duration = pulseIn(WATER_ECHO, HIGH, 40000UL);
  if (duration == 0) return NAN;

  float distanceCm = duration * 0.0343f / 2.0f;

  // Prototype tank calibration: sensor-to-bottom distance is assumed 100 cm.
  // Replace TANK_DEPTH_CM after measuring the actual tank.
  constexpr float TANK_DEPTH_CM = 100.0f;
  float level = TANK_DEPTH_CM - distanceCm;
  return constrain(level, 0.0f, TANK_DEPTH_CM);
}

float readWaterLevelFiltered() {
  float a = readRawWaterLevelCm();
  delay(30);
  float b = readRawWaterLevelCm();
  delay(30);
  float c = readRawWaterLevelCm();

  if (isnan(a) && isnan(b) && isnan(c)) return NAN;
  if (isnan(a)) a = b;
  if (isnan(b)) b = c;
  if (isnan(c)) c = b;

  return median3(a, b, c);
}

float readSoilPercent() {
  int raw = analogRead(SOIL_PIN);

  // IMPORTANT: replace these with values obtained during calibration.
  constexpr int SOIL_DRY_RAW = 3200;
  constexpr int SOIL_WET_RAW = 1300;

  float pct = 100.0f * (SOIL_DRY_RAW - raw) /
              float(SOIL_DRY_RAW - SOIL_WET_RAW);
  return constrain(pct, 0.0f, 100.0f);
}

float calculateRiseRate(float currentCm, float previousCm, float dtMin) {
  if (isnan(currentCm) || isnan(previousCm) || dtMin <= 0) return NAN;
  return (currentCm - previousCm) / dtMin;
}

float calculateWaterConfidence(float levelCm, float riseRate) {
  if (isnan(levelCm)) return 0.0f;
  float confidence = 1.0f;

  // Physically implausible one-sample jumps reduce confidence.
  if (!isnan(riseRate) && fabs(riseRate) > 25.0f) confidence -= 0.65f;
  if (levelCm < 0.0f || levelCm > 100.0f) confidence -= 0.5f;

  // Sensor should not remain exactly unchanged forever in a dynamic test.
  if (historyCount >= 5) {
    bool stuck = true;
    for (uint8_t i = 1; i < 5; i++) {
      uint8_t idxA = (historyIndex + LEVEL_WINDOW - i) % LEVEL_WINDOW;
      uint8_t idxB = (historyIndex + LEVEL_WINDOW - i - 1) % LEVEL_WINDOW;
      if (fabs(levelHistory[idxA] - levelHistory[idxB]) > 0.02f) {
        stuck = false;
        break;
      }
    }
    if (stuck) confidence -= 0.25f;
  }

  return constrain(confidence, 0.0f, 1.0f);
}

float calculateRainConfidence() {
  // Pulse-based rain measurement is intrinsically valid when interrupt wiring
  // and calibration are correct. Detailed fault detection will be added after
  // the physical rain gauge calibration experiment.
  return 1.0f;
}

float calculateSoilConfidence(float soilPct) {
  if (soilPct <= 0.0f || soilPct >= 100.0f) return 0.8f;
  return 1.0f;
}

float calculateEnvironmentConfidence() {
  return bmeAvailable ? 1.0f : 0.0f;
}

float triangular(float x, float a, float b, float c) {
  if (x <= a || x >= c) return 0.0f;
  if (x == b) return 1.0f;
  if (x < b) return (x - a) / (b - a);
  return (c - x) / (c - b);
}

float leftShoulder(float x, float a, float b) {
  if (x <= a) return 1.0f;
  if (x >= b) return 0.0f;
  return (b - x) / (b - a);
}

float rightShoulder(float x, float a, float b) {
  if (x <= a) return 0.0f;
  if (x >= b) return 1.0f;
  return (x - a) / (b - a);
}

RiskResult fuzzyRisk(float levelCm, float riseRate, float rainMmPerHour,
                     float soilPct, SensorConfidence conf) {
  float levelHigh = rightShoulder(levelCm, 55.0f, 80.0f);
  float levelCritical = rightShoulder(levelCm, 75.0f, 90.0f);

  float riseSlow = leftShoulder(fmax(riseRate, 0.0f), 0.0f, 1.0f);
  float riseFast = rightShoulder(fmax(riseRate, 0.0f), 3.0f, 8.0f);
  float rainHeavy = rightShoulder(rainMmPerHour, 10.0f, 30.0f);
  float soilWet = rightShoulder(soilPct, 60.0f, 90.0f);
  float confidence = (conf.water + conf.rain + conf.soil + conf.environment) / 4.0f;

  // Lightweight Sugeno-style rule aggregation. The constants are a research
  // baseline and will be tuned from experiments, not presented as learned AI.
  float numerator = 0.0f;
  float denominator = 0.0f;

  auto addRule = [&](float activation, float output) {
    numerator += activation * output;
    denominator += activation;
  };

  // High level + fast rise + heavy rain -> critical.
  addRule(fmin(fmin(levelHigh, riseFast), rainHeavy), 95.0f);
  // Critical level -> critical.
  addRule(levelCritical, 100.0f);
  // Medium/high level + fast rise -> warning.
  addRule(fmin(levelHigh, riseFast), 80.0f);
  // High level + wet soil -> warning/watch.
  addRule(fmin(levelHigh, soilWet), 72.0f);
  // Heavy rain + wet soil -> watch/warning.
  addRule(fmin(rainHeavy, soilWet), 65.0f);
  // Stable low-risk conditions -> safe.
  addRule(fmin(leftShoulder(levelCm, 20.0f, 45.0f), riseSlow), 15.0f);

  float fuzzy = denominator > 0.0f ? numerator / denominator : 0.0f;
  float finalRisk = fuzzy * confidence;

  String state = "SAFE";
  if (confidence < 0.40f) state = "SENSOR_FAULT";
  else if (finalRisk >= 80.0f) state = "CRITICAL";
  else if (finalRisk >= 60.0f) state = "WARNING";
  else if (finalRisk >= 35.0f) state = "WATCH";

  return {fuzzy, finalRisk, state};
}

float estimateThresholdMinutes(float levelCm, float riseRate) {
  if (isnan(levelCm) || isnan(riseRate) || riseRate <= 0.05f) return INFINITY;
  if (levelCm >= WARNING_LEVEL_CM) return 0.0f;
  return (WARNING_LEVEL_CM - levelCm) / riseRate;
}

void setLocalWarning(const String &state) {
  digitalWrite(LED_GREEN, state == "SAFE" ? HIGH : LOW);
  digitalWrite(LED_YELLOW, (state == "WATCH" || state == "WARNING") ? HIGH : LOW);
  digitalWrite(LED_RED, (state == "CRITICAL" || state == "SENSOR_FAULT") ? HIGH : LOW);

  if (state == "CRITICAL" || state == "SENSOR_FAULT") {
    tone(BUZZER_PIN, 2200, 250);
  } else if (state == "WARNING") {
    tone(BUZZER_PIN, 1600, 120);
  }
}

void setup() {
  Serial.begin(115200);
  delay(500);

  pinMode(WATER_TRIG, OUTPUT);
  pinMode(WATER_ECHO, INPUT);
  pinMode(RAIN_PIN, INPUT_PULLUP);
  pinMode(SOIL_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);

  Wire.begin(I2C_SDA, I2C_SCL);
  bmeAvailable = bme.begin(0x76);
  if (!bmeAvailable) bmeAvailable = bme.begin(0x77);

  attachInterrupt(digitalPinToInterrupt(RAIN_PIN), rainTipISR, FALLING);
  rainWindowStartMs = millis();
  previousSampleMs = millis();

  Serial.println("\n=== FloodGuard Edge-AI: Phase 1 ===");
  Serial.println("Local edge warning active; cloud layer intentionally not required.");
}

void loop() {
  uint32_t now = millis();
  if (now - previousSampleMs < SAMPLE_INTERVAL_MS) return;

  float dtMin = (now - previousSampleMs) / 60000.0f;
  previousSampleMs = now;

  float levelCm = readWaterLevelFiltered();
  float riseRate = calculateRiseRate(levelCm, previousLevelCm, dtMin);
  previousLevelCm = levelCm;

  if (!isnan(levelCm)) {
    levelHistory[historyIndex] = levelCm;
    historyIndex = (historyIndex + 1) % LEVEL_WINDOW;
    if (historyCount < LEVEL_WINDOW) historyCount++;
  }

  uint32_t tipsNow;
  noInterrupts();
  tipsNow = rainTips;
  interrupts();

  uint32_t elapsedRainMs = now - rainWindowStartMs;
  float rainMm = (tipsNow - previousRainTips) * MM_PER_TIP;
  float rainMmPerHour = elapsedRainMs > 0
      ? rainMm * (3600000.0f / elapsedRainMs)
      : 0.0f;

  // Reset the short rainfall measurement window every 5 minutes.
  if (elapsedRainMs >= 300000UL) {
    previousRainTips = tipsNow;
    rainWindowStartMs = now;
  }

  float soilPct = readSoilPercent();
  float temperature = NAN;
  float humidity = NAN;
  float pressure = NAN;

  if (bmeAvailable) {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0f;
  }

  SensorConfidence conf;
  conf.water = calculateWaterConfidence(levelCm, riseRate);
  conf.rain = calculateRainConfidence();
  conf.soil = calculateSoilConfidence(soilPct);
  conf.environment = calculateEnvironmentConfidence();

  RiskResult risk = fuzzyRisk(levelCm, riseRate, rainMmPerHour, soilPct, conf);
  float thresholdMinutes = estimateThresholdMinutes(levelCm, riseRate);

  setLocalWarning(risk.state);

  Serial.println("\n----------------------------------------");
  Serial.printf("Water Level      : %.2f cm\n", levelCm);
  Serial.printf("Rise Rate        : %.2f cm/min\n", riseRate);
  Serial.printf("Rainfall Rate    : %.2f mm/h\n", rainMmPerHour);
  Serial.printf("Soil Moisture    : %.2f %%\n", soilPct);
  Serial.printf("Temperature      : %.2f C\n", temperature);
  Serial.printf("Humidity         : %.2f %%\n", humidity);
  Serial.printf("Pressure         : %.2f hPa\n", pressure);
  Serial.printf("Confidence       : W %.2f R %.2f S %.2f E %.2f\n",
                conf.water, conf.rain, conf.soil, conf.environment);
  Serial.printf("Fuzzy Risk       : %.2f\n", risk.fuzzyRisk);
  Serial.printf("Final Risk       : %.2f\n", risk.finalRisk);
  Serial.printf("Warning State    : %s\n", risk.state.c_str());
  if (isinf(thresholdMinutes)) Serial.println("Threshold ETA    : not currently predictable");
  else Serial.printf("Threshold ETA    : %.2f min\n", thresholdMinutes);
  Serial.println("----------------------------------------");
}
