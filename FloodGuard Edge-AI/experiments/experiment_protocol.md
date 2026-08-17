# FloodGuard Edge-AI — Experimental Protocol v0.1

## Objective

Generate a controlled time-series dataset to compare fixed-threshold warning against the proposed edge fuzzy sensor-fusion method.

## Experimental variables

- Water level
- Water-level rise rate
- Rainfall intensity and accumulation
- Soil moisture
- Temperature/humidity/pressure when available
- Sensor-confidence state
- Warning state

## Tank setup

Use a transparent tank approximately 60–100 cm high with:

- controlled inlet/pump
- controllable drain
- fixed JSN-SR04T mounting
- simulated rainfall/tipping bucket
- soil container holding calibrated soil sensor
- ESP32 data logger

The actual tank dimensions and danger level must be measured and documented before data collection.

## Test classes

### A — Stable/normal
Slow or zero water-level change, no significant rainfall.

### B — Gradual rise
Controlled low-rate filling.

### C — Rapid rise
High-rate filling designed to produce a short threshold-crossing lead time.

### D — Rain-driven rise
Simulated rainfall plus controlled inflow.

### E — Saturated soil
Wet/saturated soil with increasing rainfall/inflow.

### F — Drain restriction
Reduce outlet flow while applying rainfall/inflow.

### G — Falling water
Stop inflow/open drain and verify warning downgrade.

### H — Sensor fault injection
Create missing, stuck, noisy and implausible water-level measurements.

## Fault cases

1. Missing ultrasonic echo.
2. Implausible jump between consecutive measurements.
3. Constant/stuck reading.
4. Random noise/outliers.
5. Soil sensor disconnected.
6. Rain gauge pulse interruption.

## Label definitions

Labels must be defined before collecting the final test set. Proposed initial classes:

- SAFE: no immediate threshold crossing expected.
- WATCH: risk increasing but no near-term threshold crossing.
- WARNING: predicted threshold crossing within the configured warning horizon.
- CRITICAL: threshold exceeded or experimentally defined severe trajectory.
- SENSOR_FAULT: sensor confidence below the operational limit.

## Dataset split

Do not randomly mix adjacent samples from the same continuous experiment across train and test sets. Split by complete experiment/run where possible to avoid temporal leakage.

Recommended initial split:

- 60% experiments: development/training
- 20% experiments: validation/tuning
- 20% experiments: final held-out test

## Metrics

- Precision
- Recall
- F1-score
- Confusion matrix
- False-alert rate
- Missed-alert rate
- Mean/median alert lead time
- Threshold-crossing ETA error
- Edge inference latency
- RAM/flash usage
- Power consumption

## Reproducibility

For every run record:

- date/time
- tank dimensions
- starting water level
- pump/inlet setting
- drain setting
- rainfall simulation setting
- soil condition
- sensor calibration versions
- firmware commit hash
- run ID

Never invent results. The paper tables must be populated only from measured experiments.
