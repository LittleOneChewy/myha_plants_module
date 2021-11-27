# My Home Automation (MyHA).
## Plants watering and temperature sensing module.
Developed using PlatformIO.

## Purpose for the module
1. Local temperature and humidity sensor with publishing data through MQTT
2. Plants light control with light sensor and manual control via MQTT
3. Measuring plants soil level and turn on water then needed

## Module consists from:
TBD

## How it works.
TBD

## Changelog
First attempt was done in 2019, using "Bluepill", some sensors, OLED display, DC motor pumps and crappy DC adapter. After a while the second version of module was developed. In major changes:
1. Switching to ESP32 (ESP32-C3 coming)
2. Replacing DC-motors with water solenoid valve and direct connection to water source
3. Replacing OLED display with 8x2 LCD
4. Replacing relays with mosfet to control LED power and brightness
5. Replacing notebook-type DC adapter with more suitable one, which provides independent standby 5v@0.3A, power on control and stable 12v@12A output
