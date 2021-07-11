This repo is an extended version of https://github.com/easytarget/esp32-cam-webserver with a mashup of several projects to form a single "IP Camera" out of a simple ESP32-CAM. What's different about this repo than others is that this version contains a built in WiFi Manager, OTA updates, and Multiclinet (Up to 10) streaming.  It also has image adjustment filters for the OV2640 and a few others.

Repos used:
https://github.com/me-no-dev/ESPAsyncWebServer
https://github.com/ayushsharma82/AsyncElegantOTA
https://github.com/tzapu/WiFiManager

Repos that provided either code or significant help:
https://github.com/easytarget/esp32-cam-webserver (This is literally what this entire repo started as)
https://github.com/arkhipenko/esp32-cam-mjpeg-multiclient (Huge amount of work done by arkhipenko for multi client streaming)


TODO List
Link in latest camera drivers rather from Espressif ( I only have OV2640s right now, I ordered others to start and test this)
Cleanup the code!!



FOR OTA:

Make sure you set your partioning to spiffs, for example my platformioino has:

[env:esp32cam]
platform = espressif32
board = esp32cam
board_build.partitions = default.csv
...

Using the defaults in PlatformIO will not work as they do not support OTA.