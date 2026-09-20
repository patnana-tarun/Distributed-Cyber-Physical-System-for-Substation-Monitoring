# Configuration

For the complete Blynk setup, see [Blynk Setup](blynk.md). It covers templates, datastreams, dashboard widgets, terminal commands, and device tokens.

## ESP32

Edit the local copy of [`codes/node_2_esp.txt`](../../codes/node_2_esp.txt) and replace:

- `REPLACE_WITH_BLYNK_AUTH_TOKEN`
- `REPLACE_WITH_WIFI_SSID`
- `REPLACE_WITH_WIFI_PASSWORD`

Do not commit those replacements. The file is currently a source snapshot and does not include an Arduino, ESP-IDF, or PlatformIO build project.

## RP2040

Edit the local copy of [`codes/node_3_pico.txt`](../../codes/node_3_pico.txt) and replace the Blynk token, Wi-Fi values, and the sample access PIN. Use a unique PIN for each real user when the access-control implementation is deployed.

The Pico source expects MicroPython modules such as `network`, `machine`, and optionally `blynklib`. A deployable `main.py` and dependency instructions still need to be added to make flashing reproducible.

## STM32

The STM32 source references `main.h` and `font5x7.inc`, which are not included in this snapshot. Add the matching STM32CubeIDE project and generated HAL files before attempting a build.

## Credential Rotation

If a real token, Wi-Fi password, or access PIN has ever been committed or shared, revoke or change it. Removing the value from a later commit does not invalidate an already exposed credential.

## Missing Before Deployment

- Add the real ESP32 and Pico build/deployment projects.
- Add the missing STM32 headers and generated HAL project files.
- Decide whether the ESP32 and Pico should use separate Blynk templates. Separate templates are recommended because their virtual-pin protocols are different.
- Add a local, ignored configuration mechanism if credentials should not be edited directly into source files.
- Add a test procedure for Wi-Fi loss, Blynk reconnect, relay defaults, gate behavior, and motor emergency stop.
- Set `BLYNK_INSECURE = False` or replace the Pico Blynk transport before using it on an untrusted network. Verify that the selected MicroPython library supports the required TLS connection first.