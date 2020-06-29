//
//  MyLCD.h
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//

#ifndef _MY_LCD_H_

#define _MY_LCD_H_

extern const char MU;
extern const char TUBE_LEFT;
extern const char TUBE_MIDDLE;
extern const char TUBE_RIGHT;
extern const char GOING_UP;
extern const char GOING_DOWN;
extern const char NOT_SIGNIFICANT;
extern const char STABLE;

extern LiquidCrystal lcd;

void init_my_lcd();

#endif
