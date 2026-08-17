# FloodGuard Edge-AI — Research Plan

## Working title

**FloodGuard Edge-AI: Edge-Based Multi-Sensor Fuzzy Intelligence for Predictive Flood Early Warning Under Sensor Uncertainty**

## Research question

Can a lightweight edge-resident multi-sensor fuzzy inference system provide earlier and more reliable flood-risk warnings than a conventional fixed-threshold system when measurements are noisy, missing or inconsistent?

## Proposed contribution

1. Low-cost edge-resident fuzzy flood-risk inference.
2. Sensor-confidence-aware risk fusion.
3. Pre-threshold threshold-crossing lead-time estimation.
4. Local warning operation without cloud connectivity.
5. Controlled sensor-fault injection and evaluation.
6. Quantitative comparison with fixed-threshold and optional classical ML baselines.

## Initial literature matrix

| Area | Reference direction | Relevance |
|---|---|---|
| IoT + flash-flood prediction | FLOODWALL, IEEE Sensors Journal, 2023 | IoT sensing and predictive flood analysis |
| Edge AI | Flood prediction using IoT + ANN + edge computing, IEEE, 2020 | Supports local/edge inference |
| Fuzzy flood prediction | Digital Twin-Assisted Fuzzy Logic-Inspired Intelligent Approach, IEEE Sensors Journal, 2025 | Fuzzy hydrological reasoning |
| Fuzzy data fusion | Flood prediction based on fuzzy subset data fusion, IEEE ICT-DM, 2024 | Multi-sensor uncertainty/fusion |
| Flood IoT deployment | Design and Deployment of a Flash Flood Monitoring IoT, IEEE SMARTCOMP, 2020 | Deployment constraints |
| India + edge | IoT-Edge flood monitoring for Nagaon, IEEE ICRITO, 2022 | Indian deployment context |
| Multi-parameter IoT | IoT multi-parameter remote acquisition for flash floods, IEEE ELECOM, 2024 | Multi-sensor acquisition |
| Threshold baseline | Real-time water-level monitoring for IoT flash-flood warning, IEEE, 2019 | Conventional baseline |

## Critical novelty position

Do not claim 100% novelty. The literature already contains IoT flood monitoring, edge prediction, ANN/LSTM approaches, fuzzy prediction and fuzzy sensor fusion. The intended contribution is the specific combination of lightweight edge fuzzy inference, sensor-confidence weighting, pre-threshold lead-time estimation and controlled fault-injection evaluation on a low-cost embedded platform.

## Baselines

- Fixed water-level threshold.
- Hand-engineered weighted risk score (legacy prototype).
- Proposed fuzzy edge inference.
- Optional Decision Tree/Random Forest trained offline.

## Evaluation principle

No performance number should be reported before controlled experiments. In particular, do not claim a percentage accuracy or a fixed 20–40 minute prediction capability without data supporting it.
