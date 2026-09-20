# Blynk Setup

This guide describes the Blynk objects expected by the current ESP32 and RP2040 source snapshots. The code uses virtual pins, so the datastream numbers and value types must match exactly.

## 1. Create the Blynk Project

1. Sign in to [Blynk Console](https://blynk.cloud/).
2. Create a new template for the ESP32. Use Wi-Fi as the connection type and a name such as `Substation Scout ESP32`.
3. Open the template information panel and copy the **Template ID** and **Template Name** into the matching definitions at the top of [codes/node_2_esp.txt](../../codes/node_2_esp.txt).
4. Create a device from that template and copy its **Auth Token** into `BLYNK_AUTH_TOKEN`.
5. Create a second template for the Pico, such as `Substation Scout Pico`, unless both boards are intentionally managed under one template. The two boards use different virtual-pin meanings.
6. Create a device from the Pico template and use its token as `BLYNK_AUTH` in [codes/node_3_pico.txt](../../codes/node_3_pico.txt).

Do not put real values in a commit. The repository files intentionally contain `REPLACE_WITH_...` placeholders.

## 2. Create ESP32 Datastreams

In the ESP32 template, add these virtual datastreams:

| Datastream | Type | Range | Direction | Meaning |
| --- | --- | --- | --- | --- |
| `V4` | Integer | 0-255 | App to device | Joystick X; 128 is centered |
| `V5` | Integer | 0-255 | App to device | Joystick Y; 128 is centered |
| `V6` | Integer | 0-180 | App to device | Servo angle in degrees |

The source reads `V4` and `V5` on every update and immediately processes motor direction. It reads `V6` as text and converts it to an angle, so an integer slider or numeric input is appropriate.

## 3. Create Pico Datastreams

In the Pico template, add these virtual datastreams:

| Datastream | Type | Range | Direction | Meaning |
| --- | --- | --- | --- | --- |
| `V1` | Integer | 0-1 | App to device | Relay 1 off/on |
| `V2` | Integer | 0-1 | App to device | Relay 2 off/on |
| `V3` | Integer | 0-1 | App to device | Relay 3 off/on |
| `V4` | Integer | 0-1 | App to device | Relay 4 off/on |
| `V5` | String | Text | Both directions | Terminal command and response text |
| `V6` | Integer | 0-1 | Both directions | Gate closed/open |
| `V7` | Integer | 0-255 | Device to app | PIR activity indicator |

The Pico initializes `V1` through `V4` to zero at startup. The gate writes `0` or `1` to `V6`, and the PIR writes `0` or `255` to `V7`.

## 4. Build the Dashboards

Open the template Web Dashboard or mobile dashboard and add widgets using the matching datastreams.

### ESP32 Dashboard

- Add a joystick widget with X mapped to `V4` and Y mapped to `V5`.
- Add a slider or numeric input mapped to `V6`, minimum `0`, maximum `180`, step `1`.
- Label the controls `Drive` and `Servo angle`.

### Pico Dashboard

- Add four switches mapped to `V1`, `V2`, `V3`, and `V4`; use `0` for off and `1` for on.
- Add a text input or terminal widget mapped to `V5` for commands and responses.
- Add a switch mapped to `V6` for gate control; `0` closes and `1` opens the gate.
- Add an LED or value display mapped to `V7` for PIR activity.

Avoid using the same widget layout for both devices unless they share one deliberately designed template. `V4` and `V5` mean joystick axes on the ESP32 but relays and terminal traffic on the Pico.

## 5. Use the Pico Terminal

Send these lowercase commands through the `V5` text input:

| Command | Result |
| --- | --- |
| `status` | People count, IDs inside, gate state, and uptime |
| `logs` | Last ten access-log entries |
| `users` | Registered IDs and IDs currently inside |
| `reset` | Clears the people count and inside list |
| `clear` | Clears the terminal display by sending blank lines |
| `help` or `?` | Lists supported commands |

The source lowercases and strips the incoming text. Unknown commands return an error message on `V5`.

## 6. Configure the Devices

### ESP32

1. Install the Arduino ESP32 core or create an equivalent Arduino/PlatformIO project.
2. Install `BlynkSimpleEsp32` and `ESP32Servo`.
3. Copy the source into the project as a `.ino` or `.cpp` file after supplying the missing project scaffolding.
4. Set the template ID, template name, auth token, Wi-Fi SSID, and Wi-Fi password.
5. Build and upload with the motor disconnected for the first test.
6. Open the serial monitor at `115200` baud and verify that the device connects before testing the dashboard.

### RP2040

1. Flash a compatible MicroPython firmware to the Pico.
2. Install a `blynklib` implementation compatible with this source and the selected Blynk server protocol.
3. Copy the source to the Pico as `main.py` after checking the missing deployment scaffolding.
4. Set the Blynk token, Wi-Fi values, and local access PINs.
5. Confirm the LCD reports Wi-Fi status and the serial console prints `Blynk ready`.
6. Test relays and the gate with no connected field load.

The Pico code currently creates the Blynk client on startup only. Its Wi-Fi reconnect function does not recreate a Blynk client after a connection loss, so reconnect behavior must be tested and fixed before unattended deployment.

## 7. Events, Notifications, and Messages

The current source does not call the Blynk Events API. Blynk Events or push notifications can be added later for conditions such as an obstacle, unauthorized access, or sensor alarm, but creating an Event in the console alone will not trigger it.

For the current implementation, use the Pico `V5` terminal for text messages and the `V6`/`V7` datastreams for gate and PIR state. Add event code only after defining the event codes and rate limits in the Blynk template.

## 8. First-Test Checklist

- The device appears online in Blynk Console.
- The dashboard datastream numbers match the tables above.
- ESP32 joystick center values are near `128`, and obstacle detection stops forward motion.
- ESP32 servo input is limited to `0-180`.
- Pico relay switches default to off after boot.
- Pico `status` responds through `V5`.
- Pico gate switch follows the configured open duration.
- PIR activity changes `V7`.
- Wi-Fi loss, power restart, and invalid terminal commands have been tested.
- No real credentials, tokens, or PINs are staged in Git.