# Wiring Guide

This guide is a source-derived starting point. Verify the exact board silk-screen labels, sensor voltage, connector orientation, and power requirements before applying power.

## Quick Reference

### STM32F4

| Component signal | MCU pin | Notes |
| --- | --- | --- |
| DHT11 #1 data | PA1 | Timing-sensitive GPIO |
| DHT11 #2 data | PA0 | Timing-sensitive GPIO |
| MQ-2 analog out | PA5 / ADC1 channel 5 | Confirm output never exceeds 3.3 V |
| ACS712 analog out | PA6 / ADC1 channel 6 | Source assumes 2.5 V zero point |
| Voltage-divider midpoint | PA7 / ADC1 channel 7 | Source assumes 5:1 scale |
| OLED SCL/SDA | PB8/PB9 | I2C1, address configured as 0x3C shifted |
| Alarm relay control | PB0 | Use a transistor or relay module |
| Mode button | PB1 | Source checks active-low |
| UART TX/RX | PA9/PA10 | USART1; source is configured for 115200 |

### ESP32

| Component signal | GPIO | Notes |
| --- | --- | --- |
| L298N IN1/IN2 | 26/27 | Motor direction A |
| L298N IN3/IN4 | 14/12 | Motor direction B |
| L298N ENA/ENB | 25/33 | LEDC PWM |
| HC-SR04 TRIG/ECHO | 5/4 | Level-shift ECHO to 3.3 V |
| Robot servo | 18 | External 5 V servo supply recommended |

### RP2040

| Component signal | GPIO | Notes |
| --- | --- | --- |
| Gate servo | 15 | 50 Hz PWM; external servo supply |
| PIR output | 16 | Input |
| Relay 1-4 | 17-20 | Confirm board active-high behavior |
| LCD SDA/SCL | 2/3 | I2C1 at 400 kHz |
| UART TX/RX | 0/1 | Source uses 9600 baud |
| Keypad rows | 10-13 | Outputs |
| Keypad columns | 6, 7, 21, 22 | Pulldown inputs |

## Inter-Board UART Warning

The source configurations do not currently match:

- STM32 USART1: 115200 baud
- RP2040 UART0: 9600 baud
- ESP32: `Serial` is used for debug output; no matching inter-node protocol is implemented

Do not connect these as a shared bus until the baud rate, framing, voltage levels, direction control, and message ownership are defined. Use a USB-UART adapter for isolated bench tests first.

## ASCII Bench Diagram

```text
                         +----------------------+
                         |   STM32F4 Sensor Hub |
                         |                      |
 DHT11 #1 -------------->| PA1                 |
 DHT11 #2 -------------->| PA0                 |
 MQ-2 ------------------>| PA5 ADC             |
 ACS712 ---------------->| PA6 ADC             |
 Voltage divider ------->| PA7 ADC             |
 OLED I2C <------------->| PB8/PB9             |
 Alarm driver <----------| PB0                 |
 Button ---------------->| PB1                 |
                         +----------------------+

 +---------------------+        +----------------------+
 |       ESP32         |        |      RP2040 Pico     |
 |                     |        |                      |
 | GPIO25/33 -> L298N |        | GPIO15 -> gate servo |
 | GPIO5/4 <- HC-SR04 |        | GPIO17-20 -> relays  |
 | GPIO18 -> servo     |        | GPIO2/3 -> LCD       |
 | Wi-Fi -> Blynk      |        | GPIO10-13 -> rows    |
 +---------------------+        | GPIO6,7,21,22 -> cols|
                                | Wi-Fi -> Blynk       |
                                +----------------------+
```

## Power Rules

- Use a common signal ground only after confirming the supplies are compatible.
- Power servos, the L298N motor supply, and relay coils from suitable external supplies.
- Never route motor or relay current through a Pico or ESP32 development-board regulator.
- Add a fuse and an emergency disconnect before connecting field equipment.