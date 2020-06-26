
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

//extern const int rs = 20, en = 21, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
extern LiquidCrystal lcd;

void init_my_lcd();

#endif
