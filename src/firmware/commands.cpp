#include "eeprom_28c256.h"
#include "commands.h"
#include <Arduino.h>

void write_byte_command(uint16_t address, uint8_t data) {
  // First read in the byte currently stored at this address
  uint8_t old_byte;
  read_byte(address, &old_byte);
  write_byte(address, data);
  uint8_t new_byte;
  read_byte(address, &new_byte);

  if (new_byte == data) {
    Serial.println("success");
    Serial.println(old_byte);
    Serial.println(new_byte);
  } else {
    Serial.println("failed");
  }
}

void read_byte_command(uint16_t address) {
  uint8_t byte;
  read_byte(address, &byte);
  Serial.println(byte);
}

void write_chunk_command(uint16_t chunk_address, uint8_t* chunk_data, uint16_t chunk_size) {
  bool success = write_chunk(chunk_address, chunk_data, chunk_size);

  Serial.println(success ? "success" : "failed");
}

void memdump_command(uint16_t lower_address, uint16_t upper_address) {
  for (uint16_t address = lower_address; address <= upper_address; address++) {
    uint8_t data = 0;
    read_byte(address, &data);
    Serial.println(data);
  }
  Serial.println("ack");
}
