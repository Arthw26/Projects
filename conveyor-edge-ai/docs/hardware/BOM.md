# Hardware BOM — Rev A

## Core controller

| Qty | Part | Specification / selection | Purpose |
|---:|---|---|---|
| 1 | ST NUCLEO-H743ZI2 | STM32H743ZIT6, Cortex-M7, up to 480 MHz, 2 MB Flash, up to 1 MB RAM, on-board ST-LINK, Ethernet RJ45 | Main controller, DSP, TinyML, Ethernet dashboard |

## Sensors

| Qty | Part | Specification | Interface |
|---:|---|---|---|
| 1 | ST IIS3DWB vibration sensor kit | STEVAL-MKI208V1K; 3-axis, ±2/±4/±8/±16 g, bandwidth to 6 kHz, SPI | SPI1 |
| 1 | TI INA226 breakout | 36 V, 16-bit current/voltage/power monitor; select a breakout with a known shunt and accessible I2C pins | I2C1 |
| 1 | TI TMP117 breakout | High-accuracy digital temperature sensor | I2C1 |
| 1 | TI DRV5032DU | TO-92, 20 Hz Hall switch, 1.65–5.5 V, push-pull output | Timer/GPIO |
| 1 | Neodymium magnet | Small disc/cube magnet | RPM target for Hall sensor |

## Display and alerts

| Qty | Part | Specification | Purpose |
|---:|---|---|---|
| 1 | 0.96-inch SSD1306 OLED | I2C, 128x64, 3.3 V-compatible module | Local readings/status |
| 1 | 5 V active piezo buzzer | Low-current active buzzer | Local alarm |
| 1 | 2N2222 / PN2222A | NPN transistor | Buzzer driver |
| 3 | 5 mm LED | Green / Yellow / Red | Status indicators |
| 3 | 330 ohm resistor | 1/4 W | LED current limiting |
| 1 | 1 kohm resistor | 1/4 W | Buzzer transistor base |
| 1 | 100 kohm resistor | 1/4 W | Buzzer transistor base pulldown |

## Motor and mechanical system

| Qty | Part | Selection | Purpose |
|---:|---|---|---|
| 1 | 12 V DC geared motor | Target: ~100–130 RPM, metal gearbox, stall current preferably <= 2 A | Conveyor drive |
| 1 | Motor driver carrier | TI DRV8871-based carrier; 6.5–45 V motor supply, 3.6 A peak | Motor PWM/direction control |
| 1 | 12 V DC supply | Recommended >= 3 A | Motor power |
| 1 | Inline fuse | ~2 A initially; choose after measuring startup/stall current | Motor protection |
| 1 | Small conveyor frame | Approx. 40–60 cm belt length | Test platform |
| 2 | Rollers | Sized to belt/frame | Conveyor |
| 1 | Rubber conveyor belt | Replaceable test belt | Controlled faults |
| 1 | Coupler / pulley set | Compatible with motor shaft | Drive |

## Wiring / prototyping

- Solderless breadboard for low-current sensor/display wiring.
- Perfboard or screw-terminal board for the motor/power section.
- Dupont jumper wires and JST/screw-terminal connectors.
- 0.1 uF ceramic decoupling capacitors for sensor modules where not already populated.
- 4.7 kohm pull-up resistors for I2C SDA/SCL if the connected breakout boards do not already provide suitable pull-ups.
- 12 V to 5 V buck converter, >=1 A, if the 5 V buzzer/other 5 V accessories are powered from the 12 V rail.
- Common ground between the STM32 logic supply and motor-driver logic ground.

## Important selection rule

Buy the exact board/module variants listed above where possible. Do not substitute a sensor module with an unknown logic voltage or pinout without checking its datasheet first.
