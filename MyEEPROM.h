#include <Arduino.h>

#ifndef _MY_EEPROM_H_

#define _MY_EEPROM_H_

unsigned long eeprom_crc(void);

void init_EEPROM_value(bool force = false);
long read_EEPROM_value();

void put_EEPROM_value();
#endif
