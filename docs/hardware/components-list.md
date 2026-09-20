# Components List

This is a planning BOM derived from the current source. Verify ratings, quantities, and connector types before purchasing or assembling.

## Main Components

| Node | Component | Qty | Purpose |
| --- | --- | ---: | --- |
| STM32 | STM32F401RE Nucleo or compatible STM32F4 board | 1 | Sensor hub |
| STM32 | DHT11 | 2 | Temperature and humidity |
| STM32 | MQ-2 gas sensor | 1 | Gas-level analog signal |
| STM32 | ACS712 current sensor | 1 | Current measurement |
| STM32 | Resistor voltage divider | 1 | Voltage measurement |
| STM32 | SSD1306 128x64 OLED | 1 | Local status display |
| STM32 | Relay module or transistor-driven relay | 1 | Alarm output |
| ESP32 | ESP32 development board | 1 | Mobile controller and Wi-Fi |
| ESP32 | L298N dual H-bridge | 1 | DC motor driver |
| ESP32 | HC-SR04 ultrasonic sensor | 1 | Obstacle detection |
| ESP32 | SG90 or equivalent servo | 1 | Robot actuator |
| Pico | Raspberry Pi Pico/RP2040 board | 1 | Access controller |
| Pico | 4x4 matrix keypad | 1 | User input |
| Pico | 16x2 I2C LCD | 1 | Access status |
| Pico | SG90 or equivalent servo | 1 | Gate actuator |
| Pico | 4-channel relay module | 1 | Access-control outputs |
| Pico | PIR motion sensor | 1 | Motion indication |

## Required Supporting Hardware

- Regulated 3.3 V and 5 V supplies as required by the chosen modules
- Logic-level shifter or resistor dividers for signals above 3.3 V
- Common-ground wiring and appropriately sized conductors
- Fuses, flyback protection, terminal blocks, and an emergency disconnect
- USB-UART adapter for isolated serial testing
- Enclosure and strain relief for the final assembly

## Open Design Decisions

- Select the exact relay module polarity and isolation method.
- Define whether the nodes communicate directly or through a UART/CAN/RS-485 bridge.
- Choose a single inter-node baud rate and message protocol.
- Add the actual motor, servo, and field-load power budget.