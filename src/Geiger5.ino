#include <Arduino.h>

#include <assert.h>

#include <EEPROM.h>
#include <LiquidCrystal.h>

#include "DeltaBuffer.h"
#include "MyLCD.h"
#include "MyTools.h"
#include "Radioactivity.h"

const int geigerPin = 18;
const int lcdContrastPin = 2;
const int buttonPin = 3;

const int32_t N_TERMS = 3;
const int32_t PERCENT_EVOL_DETECT = 20;

// Advanced Geiger-Müller counter software
//
// General idea: - Displays click count or average radioactivity in the past
//                 few seconds or minutes or..., or in a past interval
//               - Cpm, uSv/h or mSv/y
//               - Remember values depending on the size of your memory
//                 1 hour = 60*60*4 = 7200 bytes with 32-bit precision

typedef unsigned long click_count_t; // 32-bit: prefer this value: safe!
//typedef unsigned int click_count_t; // 16-bit: Why-not-value if you know what your're doing!
//typedef unsigned char click_count_t; // 8-bit: risky value!!!

const bool PARANOIA = true;

click_count_t clicks; //variable for GM Tube events
unsigned long orgMillis; //variable for measuring time
unsigned long targetMillis; //variable for measuring time

// Geiger-Müller: we count clicks, and that's it!
void impulse() {
  clicks++;
}

const unsigned long ONE_uSv_PER_HOUR_IN_CPM = 151; // Tube dependend!
const float CONVERT_CPM_TO_uSv_PER_HOUR = 60.0 / ONE_uSv_PER_HOUR_IN_CPM;
const float CONVERT_uSv_PER_HOUR_TO_mSv_PER_YEAR = 365.25 * 24 / 1000;
const float CONVERT_CPM_TO_mSv_PER_YEAR = CONVERT_CPM_TO_uSv_PER_HOUR * CONVERT_uSv_PER_HOUR_TO_mSv_PER_YEAR;

#define STORE_MINUTES_IN_EEPROM

#ifdef STORE_MINUTES_IN_EEPROM

#include "MyEEPROM.h"

uint32_t minute_count = 0;

uint32_t next_objective = 1;
uint32_t factor = 1; // then 2, 4, 8, ...

#endif

int display_mode = 2; // 0 = cpm, 1 = uSv/h, 2 = mSv/year
int new_display_mode = display_mode; 

int n_different_buffers = 0;
int different_buffers_index[N_TERMS];

// Short-term, mid-term, long-term (in seconds)
float unit_coeffs[N_TERMS] = { 60.0, CONVERT_CPM_TO_uSv_PER_HOUR, CONVERT_CPM_TO_mSv_PER_YEAR };
const char* units[] = { "cpm", "\001Sv/s", "mSv/y" };
int digits[] = { 2, 2, 2 };

#define SAVE_MEMORY

#ifdef SAVE_MEMORY

//int32_t duration_windows[N_TERMS] = { 10, 3*60, 10*60 };
int32_t duration_windows[N_TERMS] = { 60, 3*60, 10*60 };
int32_t granularities[N_TERMS] = { 1, 15, 60 };

DeltaBuffer<click_count_t> short_term_buf(max(60, 2 * duration_windows[0] / granularities[0]));
DeltaBuffer<click_count_t> mid_term_buf(2 * duration_windows[1] / granularities[1]);
DeltaBuffer<click_count_t> long_term_buf(2 * duration_windows[2] / granularities[2]);

DeltaBuffer<click_count_t>* buffers[N_TERMS] = { &short_term_buf, &mid_term_buf, &long_term_buf };

#else

// Works well with a Mega2560 where we'got plenty of memory!
int32_t duration_windows[N_TERMS] = { 10, 1*60, 3*60 };
int32_t granularities[N_TERMS] = { 1, 1, 1 };

DeltaBuffer<click_count_t> one_buf(2 * duration_windows[N_TERMS - 1]);
DeltaBuffer<click_count_t>* buffers[N_TERMS] = { &one_buf, &one_buf, &one_buf };

#endif

int buttonVal;

void buttonChanged() {
  int val = digitalRead(buttonPin);

  if (val != buttonVal && val == HIGH) {      
    delayMicroseconds(5000); // works, as well as millis(): but will never be increased!
    // Debouncing!

    val = digitalRead(buttonPin);

    if (val == HIGH) {
      // Confirmed!
      new_display_mode = normalize(display_mode + 1, 3);
    }
  }
  
  buttonVal = val;
}

void setup() {
  // Start counting clicks
  clicks = 0;
  
  // Pin connected to Geiger board
  pinMode(geigerPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(geigerPin), impulse, FALLING); //define external interrupts

  // Ready to start counting:
  orgMillis = millis();
  targetMillis = orgMillis + 1000;
    
  // Push button change:
  pinMode(buttonPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(buttonPin), buttonChanged, CHANGE);

  // LCD init:
  init_my_lcd();

  Serial.begin(9600);

  // Debug
  Serial.println(F("Start counter"));

  // Detect if we are reusing a buffer several times
  // and create an index of indices that are not reused
  // Needed to avoid incrementing a buffer several times!
  for (int8_t i1 = 0; i1 < N_TERMS; i1++) {
    int8_t i2 = 0;

    while (i2 < n_different_buffers && buffers[different_buffers_index[i2]] != buffers[i1]) {
      i2++;
    }

    if (i2 == n_different_buffers) {
      different_buffers_index[n_different_buffers] = i1;
      n_different_buffers++;
    }
  }

  // Wait until ready to display!
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  
  // Debug info:
  Serial.print("Different buffers: ");
  Serial.println(n_different_buffers);
  for (int8_t j = 0; j < n_different_buffers; j++) {
    Serial.println(different_buffers_index[j]);
  }
  
  // The following calculationn shows that you need to store 32-bit values
  // if you may be facing high values of radiation...
  
  // Saturation at approx. 1mSv/h
  unsigned long saturation_at_uSv_per_hour = 1000;
  unsigned long saturation_at_cpm = saturation_at_uSv_per_hour * ONE_uSv_PER_HOUR_IN_CPM;
  unsigned long max;
  switch (sizeof(click_count_t)) {
    case 1: max = 0xFF;
      break;
    case 2: max = 0xFFFF;
      break;
    case 4: max = 0xFFFFFFFF;
      break; 
  }

  float saturation_after = float(max) / saturation_at_cpm * 60;
  Serial.print(sizeof(click_count_t)*8);
  Serial.print(F("-bit oriented: Will overflow after "));
  Serial.print(saturation_after);
  Serial.print(F(" seconds of saturation at "));
  Serial.print(saturation_at_uSv_per_hour);
  Serial.println(F(" uSv/h..."));

#ifdef STORE_MINUTES_IN_EEPROM
  init_EEPROM_value();

  lcd.clear();
  lcd.print("Tube use: ");
  lcd.print(minute_count);
  lcd.print("'");
  lcd.setCursor(0, 1);
  lcd.print("\002\003\003\003\003\003\003\003\003\003\003\003\003\003\003\004");
#endif
}

void loop() {
  int8_t i;
  unsigned long currentMillis = millis();
  float v[N_TERMS];
  float short_term_radioactivity_in_mSv_per_year;

  // We save the click value for a block of BLOCK_SIZE seconds
  if (currentMillis >= targetMillis) { // n seconds in ms
    // Executed every second:
    unsigned second_number = (currentMillis - orgMillis) / 1000;
      
    targetMillis += 1000;

    for (i = 0; i < n_different_buffers; i++) {
        if (second_number % granularities[different_buffers_index[i]] == 0) {
            Serial.print("Doing the seconds % X == 0 for i = ");
            Serial.println(i);
            Serial.println(second_number);
            buffers[different_buffers_index[i]]->add(clicks);
        }
    }
    
#ifdef STORE_MINUTES_IN_EEPROM
    if ((currentMillis / 1000) % 60 == 0) {
      minute_count++;

      if (minute_count >= next_objective) {
        Serial.println("Writing to EEPROM");
        
        put_EEPROM_value();
        next_objective = minute_count + factor;
        factor *= 2; // Just to avoid writing to the EEPROM too often!
        // We'll be writing after 1 min, 2 min, 4 min, 8 min, 16 min, etc.
      }
    }
#endif
    // Good place to change the display mode!
    display_mode = new_display_mode;

    // Extensive debug:
    for (i = 0; i < N_TERMS; i++) {
      Serial.print(i);
      Serial.print(" --> ");
      Serial.print(buffers[i]->getIndex());
      Serial.print(" ");
      Serial.print(clicks);
      Serial.print(" ");
      Serial.print(buffers[i]->getNRead());
      Serial.print(" ");
      Serial.print(buffers[i]->count_last(1));
      Serial.print(" ");
      Serial.print(buffers[i]->count_last(10));
      Serial.print(" ");
      Serial.print(buffers[i]->count_last(60)); // cpm!
      Serial.print(" ");
      Serial.print(buffers[i]->count_last(10*60));
      Serial.print(" ");
      Serial.print(buffers[i]->count_last(10*60));
      Serial.print(" ");
      Serial.print(buffers[i]->avg_last(10*60) * 60, 4);
      Serial.print(" cpm (hour avg) ");
      Serial.print(buffers[i]->avg_last(10*60) * CONVERT_CPM_TO_uSv_PER_HOUR, 4);
      Serial.print(F(" uSv/h "));
      
      Serial.print(buffers[i]->avg_last(10*60) * CONVERT_CPM_TO_uSv_PER_HOUR * (365.25 * 24 / 1000), 4);
      Serial.print(F(" mSv/an "));
      
      Serial.print(buffers[i]->avg_last(10*60) * CONVERT_CPM_TO_mSv_PER_YEAR, 4);
      Serial.print(F(" mSv/an (v2) + "));
      Serial.print(buffers[i]->avg_last(1) * 60, 4);
      Serial.print(F(" mSv/an (v2)"));
      
      Serial.println();

      if (buffers[i]->hasSignificant(1)) {
        v[i] = buffers[i]->avg_last(duration_windows[i] / granularities[i]) / granularities[i] * unit_coeffs[display_mode];
        if (i == 0) {
          // Short-term value in mSv/year
          short_term_radioactivity_in_mSv_per_year = buffers[i]->avg_last(duration_windows[i] / granularities[i]) / granularities[i] * CONVERT_CPM_TO_mSv_PER_YEAR;
        }
      } else {
        v[i] = NAN; // Not a number = unknown value!
      }
    }

    ///////////////////////////////////////////////////////////////////
    // Displaying

    // Positions of displayed values:
    const static int8_t x[N_TERMS] = { 0, 6, 12 };
    const static int8_t y[N_TERMS] = { 0, 0, 0 };

    // To force blinking
    short_term_radioactivity_in_mSv_per_year *= 100;

    lcd.clear();
    // 3 values: short, mid and long term
    for (i = 0; i < N_TERMS; i++) {
      lcd.setCursor(x[i], y[i]);
      if (isnan(v[i])) {
        // No value:
        lcd.print("----");
      } else {
        lcd.print(formatString(v[i], digits[display_mode], 4));
      }
    }

    // Up, nothing, down indicators
    for (i = 1; i < N_TERMS; i++) {
      lcd.setCursor(x[i] - 1, y[i]);

      int32_t base_calc = duration_windows[i] / granularities[i];
      
      if (!buffers[i]->hasSignificant(base_calc)) {
        lcd.print(NOT_SIGNIFICANT);
        //lcd.print('?');
      } else if (buffers[i]->hasSignificant(2 * base_calc)) {
        click_count_t val_p1 = buffers[i]->count_between(-2 * base_calc, -base_calc);
        click_count_t val_p2 = buffers[i]->count_between(-base_calc, 0);
        
        if (val_p2 >= (100 + PERCENT_EVOL_DETECT) * val_p1 / 100) {
          lcd.print(GOING_UP);
        } else if (val_p2 <= (100 - PERCENT_EVOL_DETECT) * val_p1 / 100) {
          lcd.print(GOING_DOWN);
        } else {
          lcd.print(STABLE);
        }
      } else {
        lcd.print(" ");
      }
    }

    //v1:
/*    // Units + nice drawing of a tube
    lcd.setCursor(0, 1);
    lcd.print(justify(units[display_mode], 6));

    lcd.setCursor(6, 1);
    if (short_term_radioactivity_in_mSv_per_year >= DOSES[0]) {
      // Look for what dose:
      int i = 1;

      while (i < N_DOSES && DOSES[i] < short_term_radioactivity_in_mSv_per_year) {
        i++;
      }

      // Blink!
      if (second_number % 3 != 0) {
        lcd.print(DOSE_SHORT_TEXTS[i - 1]);
      } else {
        lcd.print(frise(' ', 10));
      }
    } else {
      //lcd.print("\002\003\003\003\003\003\003\003\003\004");
      const static int tube_length = 10; // in characters
      String min_count = String();
      min_count.reserve(tube_length);

      min_count += String(minute_count);
      min_count += '\'';
      min_count = TUBE_LEFT + justify(min_count, JUSTIFY_CENTER, tube_length - 2, TUBE_MIDDLE) + TUBE_RIGHT;

      lcd.setCursor(6, 1);
      lcd.print(min_count);
    }*/

    // v2:
    // Blink 3s with a warning / 1s with units + minute count
    lcd.setCursor(0, 1);
    if (short_term_radioactivity_in_mSv_per_year >= DOSES[0] && second_number % 4 != 0) {
      // Look for what dose:
      int i = 1;

      while (i < N_DOSES && DOSES[i] < short_term_radioactivity_in_mSv_per_year) {
        i++;
      }

      // Blink!
      lcd.print(DOSE_LONG_TEXTS[i - 1]);
    } else {
      lcd.setCursor(0, 1);
      lcd.print(justify(units[display_mode], 6));

      lcd.setCursor(6, 1);
      //lcd.print("\002\003\003\003\003\003\003\003\003\004");
      const static int tube_length = 10; // in characters
      String min_count = String();
      min_count.reserve(tube_length);

      min_count += String(minute_count);
      min_count += '\'';
      min_count = TUBE_LEFT + justify(min_count, JUSTIFY_CENTER, tube_length - 2, TUBE_MIDDLE) + TUBE_RIGHT;

      lcd.setCursor(6, 1);
      lcd.print(min_count);
    }
  }
}
