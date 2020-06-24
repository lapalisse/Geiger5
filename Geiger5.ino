#include <EEPROM.h>
#include <LiquidCrystal.h>

#include "DeltaBuffer.h"
#include "DeltaBuffer.cpp"

const int geigerPin = 18;
const int lcdContrastPin = 2;

const int32_t N_TERMS = 3;
const int32_t PERCENT_EVOL_DETECT = 20;

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

byte not_significant_details[8] = {
  B00100,
  B01010,
  B10001,
  B00000,
  B10001,
  B01010,
  B00100,
  B00000,
};

const char MU = '\001';
const char TUBE_LEFT = '\002';
const char TUBE_MIDDLE = '\003';
const char TUBE_RIGHT = '\004';
const char GOING_UP = '\005';
const char GOING_DOWN = '\006';
const char NOT_SIGNIFICANT = '\007';

// mSv/year
const float EFFECTIVE_DOSE      = 1; 
const float SEARCH_COVER_DOSE   = 10;
const float NUCLEAR_WORKER_DOSE = 20;
const float GO_AWAY_DOSE        = 50; 
const float HIGH_DOSE           = 100;
const float DEADLY_DOSE         = 1000;

const int N_DOSES = 6;

const float DOSES[N_DOSES] = { 
  EFFECTIVE_DOSE, SEARCH_COVER_DOSE, NUCLEAR_WORKER_DOSE, GO_AWAY_DOSE, HIGH_DOSE, DEADLY_DOSE
};

const char* DOSE_TEXTS[N_DOSES] = {
  ">Effective",
  ">SrchCover",
  ">NucWorker",
  ">Go away  ",
  ">High     ",
  ">Deadly   "
};

// Slightly more sophisticated Geiger-Müller counter software
//
// General idea: - Displays click count or average radioactivity in the past
//                 few seconds or minutes or..., or in a past interval
//               - Cpm or uSv/h
//               - Remember values depending on the size of your memory
//                 1 hour = 60*60*4 = 7200 bytes with 32-bit precision

typedef unsigned long click_count_t; // 32-bit: prefer this value: safe!
//typedef unsigned int click_count_t; // 16-bit: Why-not-value if you know what your're doing!
//typedef unsigned char click_count_t; // 8-bit: risky value!!!

const bool PARANOIA = true;

// We store all click sums during every second in an array
// Danger! The amount of memory available on your Arduino
//         may very well be limited!
// Example: 1 hour = 60*60 = 3600 seconds
const unsigned long MEMORY_SIZE = 10 * 60 + 1; // +1 is important!!
const unsigned long BLOCK_SIZE = 1; // Number of seconds : 2 = storing every 2 seconds!
click_count_t values[MEMORY_SIZE];
long n_read_values;
long values_index;

click_count_t counts; //variable for GM Tube events
unsigned long orgMillis; //variable for measuring time
unsigned long targetMillis; //variable for measuring time

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

click_count_t count_clicks_since_last_seconds(long n_seconds) {
  if (PARANOIA && n_seconds / BLOCK_SIZE > MEMORY_SIZE - 1) {
    Serial.println(F("Warning! count_clicks_since_last_seconds(): n_seconds too big! Trying to continue with a smaller value..."));
    n_seconds = (MEMORY_SIZE - 1) * BLOCK_SIZE;
  }
  
  return values[normalize(values_index - 1, MEMORY_SIZE)] - values[normalize(values_index - 1 - n_seconds / BLOCK_SIZE, MEMORY_SIZE)];
}

float avg_clicks_per_second_since_last_seconds(long n_seconds = MEMORY_SIZE - 1) {
  if (PARANOIA && n_seconds / BLOCK_SIZE > MEMORY_SIZE - 1) {
    Serial.println(F("Warning! avg_clicks_per_second_since_last_seconds(): n_seconds too big! Trying to continue with a smaller value..."));
    n_seconds = (MEMORY_SIZE -1) * BLOCK_SIZE;
  }
  
  click_count_t min_possible_value = min(n_read_values, n_seconds);

  return float(count_clicks_since_last_seconds(min_possible_value)) / min_possible_value;
}

void impulse() {
  counts++;
}

const int32_t powerOf10[] = { -1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 1000000000 };
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

String justify(const String text, const int mode = JUSTIFY_LEFT, const int n_characters = 0, const char padding = ' ') {
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

String formatString(const float value, const int n_digits, const int n_characters, const int mode = JUSTIFY_LEFT|ADD_SPACES, const char fill_in_character = ' ') {
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

const unsigned long ONE_uSv_PER_HOUR_IN_CPM = 151; // Tube dependend!
const float CONVERT_CPM_TO_uSv_PER_HOUR = 60.0 / ONE_uSv_PER_HOUR_IN_CPM;
const float CONVERT_uSv_PER_HOUR_TO_mSv_PER_YEAR = 365.25 * 24 / 1000;
const float CONVERT_CPM_TO_mSv_PER_YEAR = CONVERT_CPM_TO_uSv_PER_HOUR * CONVERT_uSv_PER_HOUR_TO_mSv_PER_YEAR;

#define STORE_MINUTES_IN_EEPROM

#ifdef STORE_MINUTES_IN_EEPROM
// EEPROM handling
// We'll be writing the number of minutes the tube has been used, this way
// we can retrieve the value and add the current value, to approximate the
// time it has been used!
// We just store:
// +---+---+---+---+---+---+---+---+
// | G | M | 0 | 1 | minutes       |
// +---+---+---+---+---+---+---+---+
// 4 bytes for a magic number,
// then 4 bytes for the number of minutes as 32-bit value
const int EEPROM_DATA_ADDRESS = 0;
const uint32_t MAGIC_GM01 = 0x474D3031; // Which stands for "GM01" in ascii
uint32_t minute_count = 0;
uint32_t next_objective = 1;
uint32_t factor = 1; // then 2, 4, 8, ...

unsigned long eeprom_crc(void) {

  const unsigned long crc_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

  unsigned long crc = ~0L;

  for (int index = 0 ; index < EEPROM.length()  ; ++index) {
    crc = crc_table[(crc ^ EEPROM[index]) & 0x0f] ^ (crc >> 4);
    crc = crc_table[(crc ^ (EEPROM[index] >> 4)) & 0x0f] ^ (crc >> 4);
    crc = ~crc;
  }
  
  return crc;
}

void init_EEPROM_value(bool force = false) {
  unsigned long magic;

  EEPROM.get(EEPROM_DATA_ADDRESS, magic);
  if (force == true || magic != MAGIC_GM01) {
    // Init with the magic number then a 32-bit 0
    Serial.println("Initializing EEPROM to 0...");
    
    EEPROM.put(EEPROM_DATA_ADDRESS, MAGIC_GM01);
    EEPROM.put(EEPROM_DATA_ADDRESS + 4, minute_count);
  } else {
    EEPROM.get(EEPROM_DATA_ADDRESS + 4, minute_count);
  }

  Serial.print("Minutes from EEPROM = ");
  Serial.print(minute_count);
  Serial.println();

  next_objective = minute_count + factor;
}

long read_EEPROM_value() {
  EEPROM.get(EEPROM_DATA_ADDRESS + 4, minute_count);
}

void put_EEPROM_value() {
  EEPROM.put(EEPROM_DATA_ADDRESS + 4, minute_count);
}
#endif

const int rs = 20, en = 21, d4 = 4, d5 = 5, d6 = 6, d7 = 7;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
int display_mode = 2; // 0 = cpm, 1 = uSv/h, 2 = mSv/year

int n_different_buffers = 0;
int different_buffers_index[N_TERMS];

// Short-term, mid-term, long-term (in seconds)
float unit_coeffs[N_TERMS] = { 1.0, CONVERT_CPM_TO_uSv_PER_HOUR, CONVERT_CPM_TO_mSv_PER_YEAR };
const char* units[] = { "cpm", "\001Sv/s", "mSv/y" };
int digits[] = { 0, 2, 2 };

#define SAVE_MEMORY

#ifdef SAVE_MEMORY

int32_t duration_windows[N_TERMS] = { 10, 3*60, 10*60 };
int32_t granularities[N_TERMS] = { 1, 15, 60 };

DeltaBuffer<click_count_t> short_term_buf(max(15, 2 * duration_windows[0] / granularities[0]));
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


void setup() {
  counts = 0;
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB
  }
  
  pinMode(geigerPin, INPUT);
  attachInterrupt(digitalPinToInterrupt(geigerPin), impulse, FALLING); //define external interrupts
  
  Serial.println(F("Start counter"));

  // Detect if we are reusing a buffer several times
  // and create an index of indices that are not reused
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

  Serial.print("Different buffers: ");
  Serial.println(n_different_buffers);
  for (int8_t j = 0; j < n_different_buffers; j++) {
    Serial.println(different_buffers_index[j]);
  }
  
  // Not needed:
  /*for (int8_t i = 0; i < MEMORY_SIZE; i++) {
    values[i] = 0;
  }*/
  values[MEMORY_SIZE + 1] = 0;
  values[0] = 0;

  n_read_values = 0;
  values_index = 0;

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


  Serial.println();
  Serial.println(normalize(-1, MEMORY_SIZE));

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
  
#ifdef STORE_MINUTES_IN_EEPROM
  init_EEPROM_value();

  lcd.clear();
  lcd.print("Tube use: ");
  lcd.print(minute_count);
  lcd.print("'");
  lcd.setCursor(0, 1);
  lcd.print("\002\003\003\003\003\003\003\003\003\003\003\003\003\003\003\004");
#endif

    orgMillis = millis();
    targetMillis = orgMillis + 1000;
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
    values[values_index] = counts;
    values_index = normalize(values_index + 1, MEMORY_SIZE);

    if (n_read_values != MEMORY_SIZE) {
      n_read_values++;
    }

    for (i = 0; i < n_different_buffers; i++) {
        if (second_number % granularities[different_buffers_index[i]] == 0) {
            Serial.print("Doing the seconds % X == 0 for i = ");
            Serial.println(i);
            Serial.println(second_number);
            buffers[different_buffers_index[i]]->add(counts);
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

    Serial.print("R --> ");
    Serial.print(n_read_values);
    Serial.print(" ");
    Serial.print(counts);
    Serial.print(" ");
    Serial.print(values_index);
    Serial.print(" ");
    Serial.print(count_clicks_since_last_seconds(1));
    Serial.print(" ");
    Serial.print(count_clicks_since_last_seconds(10));
    Serial.print(" ");
    Serial.print(count_clicks_since_last_seconds(60)); // cpm!
    Serial.print(" ");
    Serial.print(count_clicks_since_last_seconds(10*60));
    Serial.print(" ");
    Serial.print(count_clicks_since_last_seconds(10*60));
    Serial.print(" ");
    Serial.print(avg_clicks_per_second_since_last_seconds(10*60) * 60, 4);
    Serial.print(" cpm (hour avg) ");
    Serial.print(avg_clicks_per_second_since_last_seconds(10*60) * CONVERT_CPM_TO_uSv_PER_HOUR, 4);
    Serial.print(F(" uSv/h "));
    Serial.print(avg_clicks_per_second_since_last_seconds(10*60) * CONVERT_CPM_TO_uSv_PER_HOUR * (365.25 * 24 / 1000), 4);
    Serial.print(F(" mSv/an "));
    Serial.print(avg_clicks_per_second_since_last_seconds(10*60) * CONVERT_CPM_TO_mSv_PER_YEAR, 4);
    Serial.print(F(" mSv/an (v2)"));
    Serial.println();

    for (i = 0; i < N_TERMS; i++) {
      Serial.print(i);
      Serial.print(" --> ");
      Serial.print(buffers[i]->getIndex());
      Serial.print(" ");
      Serial.print(counts);
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
        // Option 1: redisplay previous value, or 0
        //v[i] = (i == 0)?0:v[i - 1];

        // Option 2: Clever: display "no value"
        v[i] = NAN;
      }
    }

    ///////////////////////////////////////////////////////////////////
    // Displaying

    const static int8_t x[N_TERMS] = { 0, 6, 12 };
    const static int8_t y[N_TERMS] = { 0, 0, 0 };

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
      } else if (buffers[i]->hasSignificant(2 * base_calc)) {
        click_count_t val_p1 = buffers[i]->count_between(-2 * base_calc, -base_calc);
        click_count_t val_p2 = buffers[i]->count_between(-base_calc, 0);
        
        if (val_p2 >= (100 + PERCENT_EVOL_DETECT) * val_p1 / 100) {
          lcd.print(GOING_UP);
        } else if (val_p2 <= (100 - PERCENT_EVOL_DETECT) * val_p1 / 100) {
          lcd.print(GOING_DOWN);
        } else {
          lcd.print(" ");
        }
      } else {
        lcd.print(" ");
      }
    }
       
    // Units + nice drawing of a tube
    lcd.setCursor(0, 1);
    lcd.print(units[display_mode]);

    lcd.setCursor(6, 1);
    if (short_term_radioactivity_in_mSv_per_year >= DOSES[0]) {
      // Look for what dose:
      int i = 1;

      while (i < N_DOSES && DOSES[i] < short_term_radioactivity_in_mSv_per_year) {
        i++;
      }

      // Blink!
      if (second_number % 3 != 0) {
        lcd.print(DOSE_TEXTS[i - 1]);
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
    }
  }
}
