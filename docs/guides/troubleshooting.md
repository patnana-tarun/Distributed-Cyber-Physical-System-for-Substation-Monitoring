# Troubleshooting Guide

## Hardware Issues

### OLED is blank

1. Confirm the OLED supply and common ground.
2. Check that the module uses the configured I2C address.
3. Check PB8 and PB9 wiring and pull-ups.
4. Disconnect other I2C devices and test the display alone.

### Relay, servo, or motor resets the board

Use an external supply with enough current, connect the grounds correctly, and add suppression for inductive loads. Do not power these loads from a microcontroller GPIO pin or a weak USB rail.

### ESP32 resets when the ultrasonic sensor runs

Confirm the HC-SR04 ECHO line is level-shifted to 3.3 V and that the sensor ground is common with the ESP32 ground.

## Firmware Issues

### STM32 does not compile

The current source snapshot references `main.h`, STM32 HAL headers, and `font5x7.inc`. Add the generated STM32CubeIDE project and matching include files before diagnosing application code.

### STM32 reports zero or implausible sensor values

Check ADC reference voltage, divider ratio, ACS712 sensitivity, sensor ground, and the calibration procedure. Use a multimeter to verify the analog voltage at the MCU pin.

### STM32 UART appears unresponsive

Use 115200 8N1, send a newline-terminated command, and confirm the terminal is connected to USART1 pins PA9/PA10. `HELP` is the smallest useful test.

### Pico does not show Blynk online

Confirm Wi-Fi credentials, the auth token, the installed `blynklib`, and the server/port settings. The current reconnect function does not recreate the Blynk client after a lost Wi-Fi session, so a power cycle may be required until that code is improved.

### Blynk controls operate the wrong hardware

Check the template and datastream mapping. ESP32 V4/V5 are joystick axes, while Pico V4 is relay 4 and V5 is the terminal. Do not share a dashboard layout between the two templates without remapping it.

### Pico gate or relay moves at boot

Disconnect the mechanical load, verify the output default states, and test the relay board active-high or active-low behavior. Add an explicit safe-output initialization before attaching the load.

## Integration Issues

The current UART settings are not compatible between all nodes: STM32 uses 115200 and RP2040 uses 9600. Do not connect them directly until the protocol and baud rate are standardized. The ESP32 source has no completed inter-node UART handler in this snapshot.

## Reporting a Problem

Include the board revision, wiring table used, power supply details, source revision, serial output, and the smallest repeatable test. Never include Wi-Fi passwords, Blynk tokens, or access PINs in an issue.