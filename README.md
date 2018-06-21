# RemoteControlledGreenhouse

This project allows you to control with the OSC protocol a greenhouse.
The greenhouse is automated, so if the hygrometer marks a soil humidity that is too low the solenoid valve is opened to water,
if the temperature or humidity is too high the fans are activated and so on (the minimum and maximum levels are they can set).
The greenhouse communicates with a server to set the time ([NTPClient](https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266WiFi/examples/NTPClient/NTPClient.ino)) and allow the user to add programmed waterings.
The light can be automatic, so it lights up in the dark, or manually
## Components
  * 2 Fan
  * solenoid valve
  * led
  * hygrometer
  * DHT11 humidity and temperature sensor
  * water level sensor
  * photoresistor
  * relay
  * esp8266
  * power supply
## Links
  * [GREENHOUSE SERVER](https://github.com/Maerk/RemoteControlledGreenhouseServer)
  * [OSC PROTOCOL](https://en.wikipedia.org/wiki/Open_Sound_Control)
## Used libraries
  * [ESP8266WiFi](https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WiFi)
  * [OSC CNMAT](https://github.com/CNMAT/OSC)
  * [Adafruit Sensor](https://github.com/adafruit/Adafruit_Sensor)
  * [DHT sensor library](https://github.com/adafruit/DHT-sensor-library)
## License
This Source Code Form is subject to the terms of the Mozilla Public
License, v. 2.0. If a copy of the MPL was not distributed with this
file, You can obtain one at [http://mozilla.org/MPL/2.0/](http://mozilla.org/MPL/2.0/)

