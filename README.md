# CameraFlash

I bought a Sony a7III a while ago thinking I would be fine with no flash. I stand by this, but I do want a flashlight for video. Might as well put in a flash function while I'm at it.

### Hardware

ATTINY202 with a LM2759 led driver. V1.2 has a batterry charger that too small for me to solder by hand so I'm looking for a new one right now.

### Software

Theres only 2 kB of program memory so it cant really be all that complicated. uC sets LED current through i2c. Buttons are pin interrupts. Working on putting the uC to sleep while idle right now.