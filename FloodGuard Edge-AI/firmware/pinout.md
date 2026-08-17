# FloodGuard Edge-AI — ESP32 Pin Map

| Device | Signal | ESP32 | Notes |
|---|---|---:|---|
| JSN-SR04T | TRIG | GPIO5 | Digital output |
| JSN-SR04T | ECHO | GPIO18 | Digital input; verify voltage level for the chosen module |
| Tipping-bucket rain gauge | Pulse | GPIO19 | Interrupt input, internal pull-up |
| Capacitive soil sensor | Analog | GPIO34 | ADC input only |
| BME280 | SDA | GPIO21 | I2C |
| BME280 | SCL | GPIO22 | I2C |
| OLED 0.96 | SDA | GPIO21 | Shares I2C bus |
| OLED 0.96 | SCL | GPIO22 | Shares I2C bus |
| Buzzer | Signal | GPIO25 | Use transistor driver if required by buzzer current |
| Green LED | Signal | GPIO26 | Series resistor required |
| Yellow LED | Signal | GPIO27 | Series resistor required |
| Red LED | Signal | GPIO14 | Series resistor required |

## Power

- ESP32: regulated 5 V through its VIN/5V input or regulated 3.3 V to 3V3 as appropriate for the board.
- BME280 and OLED: use the voltage specified by the breakout board; 3.3 V is preferred for the ESP32 bus.
- JSN-SR04T: check the exact module's supply and ECHO output before connection. Do not expose an ESP32 GPIO to a voltage above its rated input.
- All modules must share common ground.

## Important calibration items

1. Measure the actual tank depth and replace `TANK_DEPTH_CM` in the firmware.
2. Calibrate `MM_PER_TIP` against the selected rain gauge.
3. Determine dry/wet raw ADC values for the actual soil sensor and replace the firmware constants.
4. Verify the JSN-SR04T's minimum range and behavior near the water surface.
5. Add surge/transient protection and waterproof connectors before outdoor deployment.
