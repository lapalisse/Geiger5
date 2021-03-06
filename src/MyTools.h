//
//  MyTools.h
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//

#ifndef _MY_TOOLS_H_

#define _MY_TOOLS_H_

long normalize(long value, long max_excluded);

extern const int ADD_SPACES;
extern const int JUSTIFY_CENTER;
extern const int JUSTIFY_LEFT;
extern const int JUSTIFY_RIGHT;

String frise(const String pattern, const int n);
String frise(const char pattern, const int n);
String justify(const String text, const int mode = JUSTIFY_LEFT, const int n_characters = 0, const char padding = ' ');
String formatString(const float value, const int n_digits, const int n_characters, const int mode = JUSTIFY_LEFT|ADD_SPACES, const char fill_in_character = ' ');

//String formatTimeMinutes(const uint32_t time_in_minutes);

typedef enum {
  TIME_UNIT_SECONDS = 1,
  TIME_UNIT_MINUTES = 2,
  TIME_UNIT_HOURS   = 4,
  TIME_UNIT_DAYS    = 8,
  TIME_UNIT_MONTHS  = 16,
  TIME_UNIT_YEARS   = 32
} time_unit_t;

String formatTime(const uint32_t time_value, const time_unit_t units = TIME_UNIT_SECONDS);

#endif
