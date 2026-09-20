# Calibration Guide

Calibrate one subsystem at a time with the actuator load disconnected. Record the measured values and the hardware revision used.

## STM32 Analog Calibration

1. Power the STM32 and sensors from a stable supply.
2. With no current load, record the ACS712 voltage reported by the `READ` command. The current formula assumes 2.5 V at zero current and 0.185 V/A sensitivity.
3. Apply a known voltage to the divider and compare the reported value with a multimeter. The source uses `voltage = ADC voltage * 5.0`.
4. Adjust the hardware scale or source constants only after confirming that the ADC input remains below 3.3 V.
5. Expose each DHT11 to a known environment and confirm both sensor status values report `OK`.

## Threshold Calibration

The defaults are:

| Threshold | Default | Command field |
| --- | ---: | --- |
| Voltage | 2.0 V | `V` |
| Current | 2.0 A | `I` |
| MQ-2 voltage | 2.5 V | `MQ` |
| Temperature | 35 C | `T1`, `T2` |
| Humidity | 80 percent | `H1`, `H2` |

Use a complete command during testing, for example:

```text
SET:V=3.3,I=2.0,MQ=1.2,T1=30,H1=70,T2=30,H2=70
```

Then use `STATUS` to confirm the stored values and `READ` to compare live readings. The relay is asserted when any configured threshold is exceeded.

## OLED and Button Check

1. Confirm the OLED responds at the configured I2C address.
2. Confirm all four display modes render without clipping.
3. Press the active-low button and verify that automatic display cycling pauses or resumes.
4. Use `DISP:DASH`, `DISP:SENS`, `DISP:GRAPH`, and `DISP:STAT` to test each view.

## ESP32 Motor and Servo Check

1. Keep the wheels off the ground and disconnect the motor battery for the first firmware test.
2. Verify joystick center is approximately 128 on both axes.
3. Test forward, reverse, clockwise, and anticlockwise commands with a low PWM value.
4. Place an object closer than 25 cm and confirm forward motion stops.
5. Test the servo from 0 to 180 degrees without hitting a mechanical stop.

## Pico Gate and Access Check

1. Test the servo with the gate linkage disconnected.
2. Verify closed and open angles in the source before attaching the linkage.
3. Test each relay with no field load.
4. Test a valid and invalid keypad user.
5. Power-cycle the Pico and confirm the persisted count behavior only after the storage file is included in the deployable project.