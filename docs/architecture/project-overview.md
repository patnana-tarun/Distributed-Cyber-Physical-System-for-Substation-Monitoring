# Project Overview

Substation Scout is a three-node cyber-physical system for transformer monitoring, automatic protection, secure access control, cloud supervision, and robotic inspection of high-voltage yards. The detailed source of truth for the intended design is [BATCH_7_REPORT.docx](../../BATCH_7_REPORT.docx).

## Goals

- Measure transformer voltage, current, hydrogen gas concentration, and coil/oil temperatures.
- Isolate the transformer locally when safety thresholds are exceeded.
- Transfer monitoring data from the STM32 to the Pico gateway over Bluetooth.
- Publish live measurements, switching state, gate state, alerts, logs, and personnel count to Blynk.
- Authenticate employees, control the gate, log access, and detect intrusion with PIR.
- Operate a camera-equipped robotic vehicle with joystick control and ultrasonic obstacle stopping.

## Nodes

| Node | Controller | Main responsibility | Source snapshot |
| --- | --- | --- | --- |
| 1 | STM32F401RE | Transformer monitoring and protection | [codes/node_1_stm_main.txt](../../codes/node_1_stm_main.txt) |
| 2 | Raspberry Pi Pico | Control-room gateway and security | [codes/node_3_pico.txt](../../codes/node_3_pico.txt) |
| 3 | ESP32-CAM | Robotic inspection and live video | [codes/node_2_esp.txt](../../codes/node_2_esp.txt) |

## System Boundary

The report defines the intended architecture. The repository source files are implementation snapshots and are not yet complete build projects. STM32 project metadata and headers, an ESP32-CAM project, and a deployable Pico `main.py` are still required.

## Integration Status

- The report specifies HC-05 Bluetooth between Node 1 and Node 2 and HTTP/Wi-Fi from Node 2 to Blynk.
- The stored Pico source uses `UART0` at 9600 baud and the stored STM32 source uses USART1 at 115200 baud; this Bluetooth/message integration needs to be reconciled before deployment.
- The stored ESP32 source is a generic ESP32 motor/Blynk snapshot and does not yet include the report's ESP32-CAM video-streaming implementation.
- Blynk virtual-pin mappings are documented separately for the current source snapshots.

## Operating Sequence

1. STM32 measures transformer parameters and locally isolates the transformer on threshold violations.
2. STM32 sends monitoring data through the report-specified Bluetooth link.
3. Pico receives and forwards measurements to Blynk, while managing switching and security.
4. The gate authenticates employees, logs access, tracks personnel, and checks PIR intrusion.
5. ESP32-CAM streams the yard view while the vehicle uses joystick commands and ultrasonic stopping.

## Recommended Build Milestones

1. Add the missing board projects and compile each node independently.
2. Reconcile the source snapshots with the report-defined Node 1/2/3 hardware assignments.
3. Validate each board on a current-limited bench supply with loads disconnected.
4. Confirm every pin and voltage level against the wiring guide.
5. Define the Bluetooth message protocol and add automated parser tests.
6. Add enclosure, fuse, emergency-stop, and field-power documentation before connecting real equipment.
