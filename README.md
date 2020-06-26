# Geiger5
Advanced Arduino Geiger-Müller counter software.

![alt text](https://raw.githubusercontent.com/lapalisse/Geiger5/master/photos/Capture%20d%E2%80%99e%CC%81cran%202020-06-26%20a%CC%80%2013.56.04.png)

## Displays the following
1. Short term, mid term and long term radioactivity in cpm, uSv/h or mSv/y.
2. An "up" or "down" arrow sign to show if how the mid and long term values are evolving
3. The units we're using
4. The number of minutes that the tube has been wokring (saved in EEPROM, but not too often of course)
5. Threshold detection for European dangerosity values : this is blinking stuff!

It is easy to configure, so you can change the following parameters:
  - short term = 10 seconds average
  - mid term = 3 minutes average
  - long term = 10 minutes average

There is another parameter: the percentage used to show "up" or "down" arrow.
Interpret it like this: e.g. for mid term: if the radioactivity since 3 minutes is 
is 20% higher than on the period from 6 to 3 minutes ago, then an "up" arrow will be displayed.

You can also change the units used on screen by pressing a push button.

Also, the values are displayed, after sufficient data has been collected.

## Photos

![alt text](https://raw.githubusercontent.com/lapalisse/Geiger5/master/photos/IMG_1166.jpeg)

The tube has been under tension for 345 minutes.

## Memory
You'll need abount 16kb of SRAM, and as much RAM memory as you've got, because we store the values for as long as needed.

## Misc
The use of EEPROM to save the number of minutes the tube has been operating is a bit funky : the life expectancy of a tube is high.
We save the value after 1 minute, 2 minutes, 4 minutes, 8... to avoid too many EEPROM writes (you have about 100000 writes until it starts failing).

I'm working on a silly library to improve this...
