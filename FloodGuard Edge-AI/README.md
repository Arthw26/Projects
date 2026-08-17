# FloodGuard Edge-AI

## Edge-Based Multi-Sensor Fuzzy Intelligence for Predictive Flood Early Warning

FloodGuard Edge-AI is a low-cost embedded/IoT research prototype for predictive flood-risk assessment under noisy and unreliable sensor conditions.

### Research objective

Compare a conventional fixed-threshold flood warning system against a lightweight edge-resident multi-sensor fuzzy inference system that uses:

- water level
- water-level rise rate
- quantitative rainfall
- soil moisture
- environmental measurements
- sensor-confidence estimation

The edge device must continue issuing local warnings when cloud connectivity is unavailable.

### Planned warning states

- SAFE
- WATCH
- WARNING
- CRITICAL
- SENSOR_FAULT

### Hardware target

ESP32 + waterproof ultrasonic water-level sensor + tipping-bucket rain gauge + capacitive soil-moisture sensor + BME280 + OLED + buzzer/LEDs.

Optional later extensions: LoRa and GSM.

### Research methodology

1. Establish fixed-threshold baseline.
2. Implement signal filtering and sensor validation.
3. Implement sensor-confidence estimation.
4. Implement lightweight fuzzy sensor fusion on ESP32.
5. Estimate threshold-crossing lead time.
6. Collect controlled experimental time-series data.
7. Inject sensor faults and missing values.
8. Compare threshold, weighted-score, fuzzy-edge and optional ML models.
9. Evaluate F1-score, false/missed alerts, lead time, inference latency, memory and power.

### Important scope decision

The previous prototype used a binary rain detector and a pipe flow sensor. FloodGuard Edge-AI replaces binary rainfall detection with quantitative rainfall measurement and does not use a closed-pipe flow sensor as a proxy for open-channel river flow.

## Repository layout

```text
FloodGuard Edge-AI/
├── firmware/
├── dataset/
│   ├── raw/
│   └── processed/
├── ml/
├── hardware/
├── experiments/
└── docs/
```

## Status

Phase 1: architecture and firmware migration in progress.
