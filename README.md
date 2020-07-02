# Geiger5
Advanced Arduino [Geiger-Müller counter](https://en.wikipedia.org/wiki/Geiger_counter) software.

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

Detects dangerosity thresholds : they will be displayed and blinking, as in the following example:
![alt text](https://raw.githubusercontent.com/lapalisse/Geiger5/master/photos/IMG_1169.jpeg)

Using the following thresholds:
<pre>
// mSv/y
const float EFFECTIVE_DOSE      = 1; 
const float SEARCH_COVER_DOSE   = 10;
const float NUCLEAR_WORKER_DOSE = 20;
const float GO_AWAY_DOSE        = 50; 
const float HIGH_DOSE           = 100;
const float DEADLY_DOSE         = 1000;
</pre>

## Memory
You'll need abount 16kb of SRAM, and as much RAM memory as you've got. 
For the RAM, it depends for how long you'll be saving values.
Example: Long term values is based on a 10 minute basis, and values are stored every minute. This will use 10*4 = 40 bytes.
But if you want a 60 minute long term value, and will be storing every 15 seconds, that makes it: 60*4*4 = 960 bytes.
etc.

## Misc
The use of EEPROM to save the number of minutes the tube has been operating is a bit funky : the life expectancy of a tube is high.
We save the value after 1 minute, 2 minutes, 4 minutes, 8... to avoid too many EEPROM writes (you have about 100000 writes until it starts failing).

I'm working on a silly library to improve this...

## Test with a source of radiation

Just got a capsule that is meant for detecting smoke: it contains a bit of [Americium](https://en.wikipedia.org/wiki/Americium). Don't worry, it isn't really dangerous, provided that you don't eat it and wear it in your underwear. 
![alt text](https://raw.githubusercontent.com/lapalisse/Geiger5/master/photos/69zTa.jpeg)

Of course, I couldn't resist to test my Geiger-Müller counter, and here is the result:
![alt text](https://raw.githubusercontent.com/lapalisse/Geiger5/master/photos/KJpZz.jpeg)

As you can see, the values move up, and the nearest, the highest:
![alt text](https://raw.githubusercontent.com/lapalisse/Geiger5/master/photos/IMG_1226.jpeg)
