# Usage Examples

## STM32 Startup

At startup the STM32 should send:

```text
=== SYSTEM READY ===
Type HELP for commands
>
```

The expected serial settings are 115200 baud, 8 data bits, no parity, and one stop bit.

## STM32 Command Session

```text
> STATUS
STATUS:V=2.0,I=2.0,MQ=2.5,T1=35.0,H1=80,T2=35.0,H2=80,RELAY=0
> READ
READ:MQ=0.56,I=0.92,V=2.34,T1=28.0,H1=45,T2=27.5,H2=44
> DISP:SENS
ACK:DISPLAY_SENSORS
> RELAY:ON
ACK:RELAY_ON
```

Values vary with the sensors. The examples describe the response shape, not guaranteed measurements.

## STM32 Telemetry

The periodic output has this shape:

```text
DATA:MQ=0.56,I=0.92,V=2.34,T1=28.0,H1=45,T2=27.5,H2=44,D1=OK,D2=OK,RELAY=0
```

| Field | Meaning |
| --- | --- |
| `MQ` | MQ-2 ADC voltage |
| `I` | Calculated current |
| `V` | Calculated voltage |
| `T1`, `H1` | DHT11 #1 temperature and humidity |
| `T2`, `H2` | DHT11 #2 temperature and humidity |
| `D1`, `D2` | Sensor status, `OK` or `ERR` |
| `RELAY` | Current alarm relay state |

## Pico Terminal Messages

When a Blynk terminal is mapped to V5, send `status`, `logs`, `users`, `reset`, `clear`, or `help`. Responses are written back to the same V5 datastream.

## ESP32 Safety Behavior

When the ultrasonic distance is below 25 cm, the ESP32 stops the motors and prints `BLOCKED`. The obstacle check is not a substitute for a physical emergency stop.