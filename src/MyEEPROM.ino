//
//  MyEEPROM.ino
//  ArduinoTools
//
//  Created by Ludovic Bertsch on 18/06/2020.
//  Copyright © 2020 Ludovic Bertsch. All rights reserved.
//

#include <Arduino.h>

#include "MyEEPROM.h"

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

unsigned long eeprom_crc(void) {

  const static unsigned long crc_table[16] = {
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

void init_EEPROM_value(bool force) {
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
