# Geiger5
Advanced Arduino Geiger-Müller counter software.

See photos to have an idea of what it looks like...

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

## Example

++++++++++++++++++
|0.35 ^0.12 v0.15|
|mSv/y <__345'__>|
++++++++++++++++++

The tube has been under tension for 345 minutes.
