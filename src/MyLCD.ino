//
//  DeltaBuffer.hpp
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//

#include <Arduino.h>

#include "MyLCD.h"

// mu character to display uSv/s
byte mu_details[8] = {
  B00000,
  B00000,
  B10010,
  B10010,
  B10010,
  B11101,
  B10000,
  B10000,
};

// Drawing of a Geiger-Müller tube:
byte tube_left_details[8] = {
  B00001,
  B00010,
  B11100,
  B11101,
  B11100,
  B00010,
  B00001,
  B00000,
};

byte tube_middle_details[8] = {
  B11111,
  B00000,
  B00000,
  B10101,
  B00000,
  B00000,
  B11111,
  B00000,
};

byte tube_right_details[8] = {
  B10000,
  B01000,
  B00111,
  B10111,
  B00111,
  B01000,
  B10000,
  B00000,
};

// Displaying value change
byte going_up_details[8] = {
  B00100,
  B01110,
  B11111,
  B00000,
  B00000,
  B00000,
  B00000,
  B00000,
};

byte going_down_details[8] = {
  B00000,
  B00000,
  B00000,
  B00000,
  B11111,
  B01110,
  B00100,
  B00000,
};

/*byte not_significant_details[8] = {
  B00100,
  B01010,
  B10001,
  B00000,
  B10001,
  B01010,
  B00100,
  B00000,
};*/
byte not_significant_details[8] = {
  B00000,
  B00100,
  B00010,
  B00101,
  B00010,
  B00100,
  B00000,
  B00000,
};

byte stable_details[8] = {
  B00000,
  B00100,
  B00110,
  B00111,
  B00110,
  B00100,
  B00000,
  B00000,
};

const char MU = '\001';
const char TUBE_LEFT = '\002';
const char TUBE_MIDDLE = '\003';
const char TUBE_RIGHT = '\004';
const char GOING_UP = '\005';
const char GOING_DOWN = '\006';
const char NOT_SIGNIFICANT = '\007';
const char STABLE = '\000';

static const int rs = 20, en = 21, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

void init_my_lcd() {
  lcd.begin(16, 2);
  pinMode(lcdContrastPin, OUTPUT);   // sets the pin as output
  analogWrite(lcdContrastPin, 90); // contrast

  lcd.createChar(MU, mu_details);
  lcd.createChar(TUBE_LEFT, tube_left_details);
  lcd.createChar(TUBE_MIDDLE, tube_middle_details);
  lcd.createChar(TUBE_RIGHT, tube_right_details);
  lcd.createChar(GOING_UP, going_up_details);
  lcd.createChar(GOING_DOWN, going_down_details);
  lcd.createChar(NOT_SIGNIFICANT, not_significant_details);
  lcd.createChar(STABLE, stable_details);
}
