# My Home Automation (MyHA).
## Plants module.
Developed using PlatformIO.  

Module consists from:
1. Power adapter (12V 10A).
2. "PhitoLed" stripe.
3. 3 channel DC-DC step-down power distribution board with transistor keys.
4. Relay module board (4 relays).
5. Two OLED displays.
6. Temperature and Humidity sensor.
7. Motion detection sensor.
8. Wireless transmitter/reciever.
9. 3 soil humidity sensors.
10. 3 water pumps.

## How it works.
12V line powers up DC-DC converter and one of the relay modules, connected to led stripe.  
Ch1 of power distribution board gives 5v power directly to BluePill and motion detector.  
Ch2 powers up soil sensors and 3 relays to water pumps.  
Ch3 used to power up displays and temp/humidity sensor.  

...to be continued...
