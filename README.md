# musical sleepers

This repo contains prototypes for the sleepers instrument.

* esp/firmwart that runs on an esp32, uses platform.io
* pd/osc-receiver: pd patch that monitors incoming osc messages and triggers sounds. any other osc monitor will do the the job as well.
* pd/simulation: pd patch that behaves the same way as esp/firmware. it can be used to prototype the audio part, independently from the hardware part