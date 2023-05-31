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
* activate features in platform.ini, by setting defines e.g. USE_TOUCH
* /sleeper/piezo_onset id:i
* /sleeper/piezo id:i value:i
* /sleeper/touch id:i value:i
* /sleeper/angle id:i x:f y:f z:f
* /sleeper/acceleration id:i x:f y:f z:f