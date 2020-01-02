# TDM

a pocket sized groovebox.

## changelog

*12/30/19*
- screen bitmap font writer (for debug)

## hardware todo
- fix the oled -> mic noise issue (seperate audio / other ground planes)
- test the line in cutout
- test the headphone cutout (paranoid about this one, might kill the teensy)
- debug the amplifier section (determine if it is necessary)
- fix the codec data line issues
- fix the screen header order (is currently reversed)
- remove the power source and swap for a battery charger & jst connector for lipo
- castellated vias for teensy connection
- add sd card port

## firmware todo

- encapsulate the core data
- serialize to flash when changed
- get the current sample thing working with pattern triggers
- convert the screen.ino into a class
- sample to the serial flash
- pitched playback for the sampler
- setup the sleep mode
- setup the gpio sleep interrupt
- subtractive synth
- physical modeled drum

## later release

- modular synth?
- lua system?

(186+100+50+40)/5
333+200+100+80