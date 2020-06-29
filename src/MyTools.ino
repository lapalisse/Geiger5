//
//  MyTools.ino
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//
#include <Arduino.h>

#include "MyTools.h"

// Forces a value to be in an interval [0..max[
// "modulo-style"...
long normalize(long value, long max_excluded) {
  while (value < 0) {
    value += max_excluded;
  }

  while (value >= max_excluded) {
    value -= max_excluded;
  }

  return value;
}

static const int32_t powerOf10[] = { -1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };

const int ADD_SPACES = 1;
const int JUSTIFY_CENTER = 2;
const int JUSTIFY_LEFT = 4;
const int JUSTIFY_RIGHT = 8;

String frise(const String pattern, const int n) {
  String s = String();
  s.reserve(n*pattern.length());
  
  for (int i = 0; i < n; i++) {
    s += pattern;
  }

  return s;
}

String frise(const char pattern, const int n) {
  String s = String();
  s.reserve(n);
  
  for (int i = 0; i < n; i++) {
    s += pattern;
  }

  return s;
}

String justify(const String text, const int mode, const int n_characters, const char padding) {
    int remaining_length = max(n_characters - text.length(), 0);
    String result = String();
    result.reserve(max(n_characters, text.length()));
    
    if (mode & JUSTIFY_LEFT) {
      result = text;
      result += frise(padding, remaining_length);
    } else if (mode & JUSTIFY_RIGHT) {
      result = frise(padding, remaining_length);
      result += text;
    } else if (mode & JUSTIFY_CENTER) {
      int half = remaining_length / 2;
      String repeated = frise(padding, half);
      
      result = repeated;
      result += text;
      result += repeated;
      
      if (remaining_length & 1) { 
        // Odd number --> we add one more character!
        result += padding;
      }
    }

    return result;
}

String formatString(const float value, const int n_digits, const int n_characters, const int mode, const char fill_in_character) {
  String str = String(value, n_digits);
  str.reserve(n_characters);

  if (value < powerOf10[n_characters]) {
    if (str.length() > n_characters) {
      int pos = str.indexOf('.');
      
      if (pos != -1) {
        str = str.substring(0, (pos == n_characters)?pos - 1:n_characters);
      } else {
        // KO!
      }
    } else {
      // Do nothing
    }
  } else if (value < powerOf10[n_characters] * 1000) {
    str = String(value / 1000, 0);
    str += "k";
  } else if (value < powerOf10[n_characters] * 1000000) {
    str = String(value / 1000000, 0);
    str += "m";
  } else {
    str = frise("#", n_characters);
  }

  // Compléter avec des espaces?
  if (mode & ADD_SPACES) {
    String result = justify(str, n_characters, mode & ~ADD_SPACES, fill_in_character);
    
    assert(result.length() == n_characters);

    return result;
  }

  return str;
}

String formatTimeMinutes(const uint32_t time_in_minutes) {
  String result;
  
  if (time_in_minutes < 60) {
    return String(time_in_minutes) + '\'';
  } else {
    uint32_t hours = time_in_minutes / 60;
    uint32_t minutes = time_in_minutes % 60;
    
    return String(hours) + 'h' + String(minutes);
  }
}

uint32_t change_of_level[] = { 1, 60, 60, 24, 30, 12 };
const char* change_of_level_unit[] = { "s", "m", "h", "d", "month", "year" };

String formatTime(const uint32_t time_value, const time_unit_t units) {
  String result;

  switch (units) {
    case TIME_UNIT_MINUTES:
      if (time_value < 60) {
        return String(time_value) + '\'';
      } else {
        uint32_t hours = time_value / 60;
        uint32_t minutes = time_value % 60;
        
        return String(hours) + 'h' + String(minutes);
      }
      break;
  }
}
