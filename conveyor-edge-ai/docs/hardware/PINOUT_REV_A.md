# STM32 Pinout & Wiring — Rev A

Controller: **NUCLEO-H743ZI2 / STM32H743ZIT6**

This is the first controlled wiring specification. Do not wire the motor section until the motor-driver and power rails are checked with a multimeter.

## 1. STM32 signal map

| Function | Nucleo Arduino pin | STM32 MCU pin | Peripheral | Connect to |
|---|---|---|---|---|
| Vibration SPI SCK | D13 / CN7-10 | PA5 | SPI1_SCK | IIS3DWB SCL/SCK |
| Vibration SPI MISO | D12 / CN7-12 | PA6 | SPI1_MISO | IIS3DWB SDO |
| Vibration SPI MOSI | D11 / CN7-14 | PB5 | SPI1_MOSI | IIS3DWB SDA/SDI |
| Vibration CS | D10 / CN7-16 | PD14 | GPIO output | IIS3DWB CS |
| I2C SCL | D15 / CN7-2 | PB8 | I2C1_SCL | INA226 + TMP117 + OLED SCL |
| I2C SDA | D14 / CN7-4 | PB9 | I2C1_SDA | INA226 + TMP117 + OLED SDA |
| RPM input | D6 / CN10-4 | PE9 | TIM1_CH1 | DRV5032DU OUT |
| Motor PWM | D5 / CN10-6 | PE11 | TIM1_CH2 | DRV8871 IN1 |
| Motor direction | D4 / CN10-8 | PE14 | GPIO output | DRV8871 IN2 |
| Buzzer control | D3 / CN10-10 | PE13 | GPIO/PWM | 2N2222 base through 1 kohm |
| Status green | D7 / CN10-2 | PG12 | GPIO output | Green LED through 330 ohm |
| Status yellow | D8 / CN7-20 | PF3 | GPIO output | Yellow LED through 330 ohm |
| Status red | D9 / CN7-18 | PD15 | GPIO output | Red LED through 330 ohm |
| OLED / sensor power | 3V3 | 3.3 V rail | Power | All 3.3 V sensor modules |
| Logic ground | GND | GND | Power | Common logic ground |

ST documents the NUCLEO-H743ZI2 Zio pin mapping and warns that STM32H7 I/O is 3.3 V compatible, not 5 V tolerant as a generic Arduino Uno shield assumption. Verify every module's logic level before connection.

## 2. IIS3DWB vibration sensor

Use **STEVAL-MKI208V1K** or an equivalent IIS3DWB breakout with accessible 3.3 V/SPI pins.

| IIS3DWB signal | STM32 |
|---|---|
| VDD | 3V3 |
| VDDIO | 3V3 |
| GND | GND |
| SCL/SCK | PA5 / D13 |
| SDA/SDI/MOSI | PB5 / D11 |
| SDO/MISO | PA6 / D12 |
| CS | PD14 / D10 |
| INT1 | Leave unconnected for Rev A; add later for FIFO/watermark optimization |

Mount the sensor rigidly on the motor/bearing housing, not on a flexible breadboard wire. Sensor orientation must be recorded in the dataset.

## 3. I2C1 bus

Shared bus:

**PB8 (SCL) + PB9 (SDA)**

Connect all I2C devices in parallel:

### INA226
- VCC → 3V3
- GND → GND
- SCL → PB8
- SDA → PB9
- Default address target: 0x40 (verify breakout configuration)

### TMP117
- VCC → 3V3
- GND → GND
- SCL → PB8
- SDA → PB9
- Default address target: 0x48 (verify breakout configuration)

### SSD1306 OLED
- VCC → 3V3
- GND → GND
- SCL → PB8
- SDA → PB9
- Common address is 0x3C; verify the actual module before firmware configuration.

Use one pair of 4.7 kohm pull-ups to 3V3 if the connected modules do not already provide them. Avoid stacking multiple strong pull-ups.

## 4. Motor current measurement — INA226

Measure the motor supply current on the **12 V motor rail** using the INA226 breakout's shunt path.

Power path:

```text
12 V PSU +
   -> fuse
   -> INA226 VIN+
   -> INA226 VIN-
   -> DRV8871 VIN/VM motor supply
```

- 12 V PSU negative → common GND.
- Do not route motor current through an STM32 GPIO or 3.3 V rail.
- Use the exact shunt value fitted to the INA226 breakout when configuring/calibrating the driver.
- Keep high-current motor wiring physically separate from sensor signal wiring.

## 5. Temperature — TMP117

- Mount the TMP117 physically against the motor housing or bearing housing using thermal tape/thermal adhesive.
- Do not mount it beside the STM32 board and call that motor temperature.
- Keep the I2C wires short and away from the motor leads.

## 6. RPM — DRV5032DU

Recommended exact device family: **DRV5032DU**, TO-92, 20 Hz, unipolar, push-pull.

For the TO-92 DU package, verify the package orientation against the TI datasheet before soldering:

- Pin 1 VCC → 3V3
- Pin 2 GND → GND
- Pin 3 OUT2 → PE9 / D6
- Add 0.1 uF from VCC to GND close to the sensor.

Mount one small neodymium magnet on the roller or pulley and place the Hall sensor with a small repeatable air gap. One magnet per revolution gives approximately one pulse per revolution.

RPM calculation:

`RPM = pulse_count / measurement_time_seconds × 60`

## 7. Motor driver — DRV8871 carrier

Use a DRV8871-based carrier with labeled logic pins.

- Driver logic GND → common GND
- Driver IN1 → PE11 / D5 (PWM)
- Driver IN2 → PE14 / D4 (direction)
- Driver OUT1 → motor terminal 1
- Driver OUT2 → motor terminal 2
- Driver VM/VIN → fused 12 V motor supply

For forward operation, use PWM on IN1 with IN2 LOW. For stop, drive both IN1 and IN2 LOW. Reverse operation can be added later.

The DRV8871 supports 6.5–45 V motor supply and up to 3.6 A peak output, but the actual motor current and carrier thermal limits must be respected.

## 8. Buzzer driver

Do **not** drive a 5 V buzzer directly from an STM32 GPIO.

```text
STM32 PE13/D3
     |
    1k
     |
  2N2222 base
  emitter -> GND
  collector -> buzzer negative

+5 V -> buzzer positive
```

Add a 100 kohm base-emitter pulldown. If a magnetic/inductive buzzer is used, add an appropriate flyback diode; a simple piezo buzzer normally does not need one.

## 9. Status LEDs

For each LED:

`STM32 GPIO -> 330 ohm resistor -> LED anode`

`LED cathode -> GND`

Assignments:
- Green: PG12 / D7
- Yellow: PF3 / D8
- Red: PD15 / D9

## 10. Power architecture

```text
12 V DC supply
 ├── fuse ──> INA226 ──> DRV8871 ──> DC motor
 │
 └──> 5 V buck converter ──> buzzer / optional 5 V accessories

NUCLEO-H743ZI2 powered from ST-LINK USB during development
 └──> 3.3 V rail ──> IIS3DWB + INA226 logic + TMP117 + OLED + Hall sensor

ALL GROUNDS COMMON
```

Do not power the motor from the Nucleo board's 5 V/3V3 pins.

## 11. Ethernet dashboard

The NUCLEO-H743ZI2 has an on-board Ethernet PHY and RJ45 connector. Ethernet uses RMII pins on the board, so the application should reserve those pins for the network stack. The dashboard plan for Rev A is:

`STM32 + lwIP HTTP server -> Ethernet -> browser`

Firebase/cloud synchronization is a later optional extension; it is intentionally not required for the first STM32-only milestone.

## 12. First bring-up order

1. STM32 + ST-LINK only.
2. I2C bus + OLED.
3. TMP117.
4. INA226 with motor disconnected.
5. IIS3DWB SPI.
6. Hall RPM sensor.
7. LEDs + buzzer driver.
8. Motor driver without belt load.
9. Motor + current measurement.
10. Ethernet dashboard.
11. Dataset collection.
12. TinyML deployment.

Never connect the motor power stage for the first time with the STM32 board and sensor wiring unverified.
