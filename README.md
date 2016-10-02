# Chrooke's Animator and Awesome Detector

## Description
This is the code for my Animator and Awesome Detector.

This is meant to run on an Arduino board with an LED matrix. A piezo buzzer can be added for sound. The "Awesome Detector" functionality also requires an ultrasonic range finder.

My device was built with several parts from the Adafruit ADABOX 001. Specifically:
* [Adafruit Feather 32u4 Adalogger](https://www.adafruit.com/products/2795)
* [Neopixel FeatherWing](https://www.adafruit.com/products/2945)
* [Piezo Buzzer](https://www.adafruit.com/products/160)

In addition I used
* [Maxbotix Ultrasonic Rangefinder - LV-EZ1](https://www.adafruit.com/index.php?q=Maxbotix%20Ultrasonic%20Rangefinder%20-%20LV-EZ1%20&main_page=account_history_info&order_id=1206735) 
* a small project box (lost the info, sorry)

It should be possible to build a similar device using other Arduino-compatible dev boards and addressable LED pixels.

## Connections
The Neopixel Featherwing is attached to headers on the Feather, but uses pin 6 for control. The ultrasonic range finder is on pin 5 and the piezo buzzer uses pin 10.

## Required libraries
I used the following libraries for this project:
* Adafruit GFX Library
* Adafruit NeoMatrix
* Adafruit NeoPixel
* [Timer](https://github.com/JChristensen/Timer)
