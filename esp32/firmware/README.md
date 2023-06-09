# firmware

## description
this program reads and processes sensor data, finally broadcasts them via OSC.

## usage
* install platform.io `brew install platformio`
* install platform.io vs code package
* add wifi credentials in credentials.h
* enable features in platform.ini
* upload firmware `pio run -t upload`
* monitor `pio device monitor`

## dev
* enable features in platform.ini, by setting defines e.g. USE_TOUCH
* /sleeper/piezo_onset id:i
* /sleeper/piezo id:i value:i
* /sleeper/touch id:i value:i
* /sleeper/angle id:i x:f y:f z:f
* /sleeper/acceleration id:i x:f y:f z:f
* if COMBINED is defined, then all enabled sensor values will be sent as a single combined message
* it can be configured via osc: port 9000, addr: /config, values: piezoOnsetThreshold:i piezoOnsetDebounceTime:i touchThreshold:i accelOnsetThreshold:i accelDebounceTime:i