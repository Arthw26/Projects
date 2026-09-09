# Energy-Aware Multi-Sensor TinyML for Conveyor Fault Detection

STM32-based Edge-AI predictive maintenance prototype for a laboratory-scale conveyor.

## Project goal

Monitor conveyor health using vibration, motor current, temperature, and RPM. Perform signal processing, multi-sensor fusion, and lightweight TinyML inference locally on an STM32. Provide immediate local alarms and a real-time web dashboard through Wi-Fi/Firebase.

## Initial scope

- Normal operation
- Belt misalignment
- Controlled overload
- Controlled mechanical abnormality
- Local buzzer + LED + OLED
- Firebase-backed web dashboard
- Normal vs abnormal waveform visualization
- Evaluation of accuracy, latency, memory, and energy/power

## Development status

**Phase 0 — Architecture and hardware selection**

The design is being developed incrementally. Pin assignments will be treated as a controlled interface specification and updated only after the exact board and module variants are confirmed.

## Repository structure

```text
conveyor-edge-ai/
├── README.md
├── docs/
│   ├── architecture/
│   ├── hardware/
│   └── research/
├── firmware/
├── data/
├── ml/
├── dashboard/
└── tests/
```

## Important engineering rule

Do not treat example measurements as experimental results. Accuracy, latency, memory, and energy values will be measured on the actual hardware and documented with test conditions.
