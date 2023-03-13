#include "eeprom_28c256.h"
#include <Arduino.h>

void init_address_pins() {
  pinMode(ADDR_0, OUTPUT);
  pinMode(ADDR_1, OUTPUT);
  pinMode(ADDR_2, OUTPUT);
  pinMode(ADDR_3, OUTPUT);
  pinMode(ADDR_4, OUTPUT);
  pinMode(ADDR_5, OUTPUT);
  pinMode(ADDR_6, OUTPUT);
  pinMode(ADDR_7, OUTPUT);
  pinMode(ADDR_8, OUTPUT);
  pinMode(ADDR_9, OUTPUT);
  pinMode(ADDR_10, OUTPUT);
  pinMode(ADDR_11, OUTPUT);
  pinMode(ADDR_12, OUTPUT);
  pinMode(ADDR_13, OUTPUT);
  pinMode(ADDR_14, OUTPUT);
}

void set_data_pin_mode(int mode) {
  pinMode(DATA_0, mode);
  pinMode(DATA_1, mode);
  pinMode(DATA_2, mode);
  pinMode(DATA_3, mode);
  pinMode(DATA_4, mode);
  pinMode(DATA_5, mode);
  pinMode(DATA_6, mode);
  pinMode(DATA_7, mode);
}

void init_signal_pins() {
  // The signal pins need to be configured to output HIGH before they actually start outputting
  // The signal pins otherwise default to LOW, which enables the chip unwantedly and clobbers data upon init
  digitalWrite(NCE, HIGH);
  digitalWrite(NWE, HIGH);
  digitalWrite(NOE, HIGH);
  pinMode(NCE, OUTPUT);
  pinMode(NWE, OUTPUT);
  pinMode(NOE, OUTPUT);
}

void init_indicator_led() {
  pinMode(GREEN_LED, OUTPUT);
}

void led_on () {
  digitalWrite(GREEN_LED, HIGH);
}

void led_off() {
  digitalWrite(GREEN_LED, LOW);
}

void write_led(int state) {
  digitalWrite(GREEN_LED, state);
}

void write_address_pins(uint16_t address) {
  digitalWrite(ADDR_0, address & 0x1);
  digitalWrite(ADDR_1, (address >> 1) & 0x1);
  digitalWrite(ADDR_2, (address >> 2) & 0x1);
  digitalWrite(ADDR_3, (address >> 3) & 0x1);
  digitalWrite(ADDR_4, (address >> 4) & 0x1);
  digitalWrite(ADDR_5, (address >> 5) & 0x1);
  digitalWrite(ADDR_6, (address >> 6) & 0x1);
  digitalWrite(ADDR_7, (address >> 7) & 0x1);
  digitalWrite(ADDR_8, (address >> 8) & 0x1);
  digitalWrite(ADDR_9, (address >> 9) & 0x1);
  digitalWrite(ADDR_10, (address >> 10) & 0x1);
  digitalWrite(ADDR_11, (address >> 11) & 0x1);
  digitalWrite(ADDR_12, (address >> 12) & 0x1);
  digitalWrite(ADDR_13, (address >> 13) & 0x1);
  digitalWrite(ADDR_14, (address >> 14) & 0x1);
}

void write_data_pins(uint8_t data) {
  digitalWrite(DATA_0, data & 0x01);
  digitalWrite(DATA_1, (data >> 1) & 0x01);
  digitalWrite(DATA_2, (data >> 2) & 0x01);
  digitalWrite(DATA_3, (data >> 3) & 0x01);
  digitalWrite(DATA_4, (data >> 4) & 0x01);
  digitalWrite(DATA_5, (data >> 5) & 0x01);
  digitalWrite(DATA_6, (data >> 6) & 0x01);
  digitalWrite(DATA_7, (data >> 7) & 0x01);
}

uint8_t read_data_pins() {
  uint8_t data = 0;

  data |= (digitalRead(DATA_0));
  data |= (digitalRead(DATA_1) << 1);
  data |= (digitalRead(DATA_2) << 2);
  data |= (digitalRead(DATA_3) << 3);
  data |= (digitalRead(DATA_4) << 4);
  data |= (digitalRead(DATA_5) << 5);
  data |= (digitalRead(DATA_6) << 6);
  data |= (digitalRead(DATA_7) << 7);

  return data;
}

void write_page(uint16_t page_address, uint8_t* page_data, uint8_t page_size) {
  set_data_pin_mode(OUTPUT);
  // Prepare pins for a write cycle
  digitalWrite(NOE, HIGH);
  digitalWrite(NWE, HIGH);
  digitalWrite(NCE, HIGH);

  for (uint8_t offset = 0; offset < page_size; offset++) {
    uint16_t address = page_address  + offset;
    uint8_t data = *(page_data + offset);
    write_address_pins(address);
    write_data_pins(data);

    // WE falling edge latches address
    // WE rising edge latches data
    digitalWrite(NCE, LOW);
    digitalWrite(NWE, LOW);
    asm(NOP);
    asm(NOP); // Very short delay (2 * ~62.5ns) to be certain data latches
    digitalWrite(NWE, HIGH);
    digitalWrite(NCE, HIGH);
  }

  // A write cycle takes anywhere between 6ms - 10ms total. 5ms appears to be too short of a delay
  // New data cannot be written until the previous cycle has completed
  delay(6);
}

bool write_chunk(uint16_t chunk_address, uint8_t* chunk_data, uint16_t chunk_size) {
  uint8_t num_pages = 1 + (chunk_size >> 6);

  for (uint16_t offset = 0; offset < chunk_size; offset+=64) {
    uint16_t page_address = chunk_address + offset;
    uint8_t* page_data = chunk_data + offset;
    uint8_t page_size = chunk_size - offset > 64 ? 64 : chunk_size - offset;
    write_page(page_address, page_data, page_size);
    if (!verify_page(page_address, page_data, page_size)) {
      return false;
    }
  }
  return true;
}

void read_byte(uint16_t address, uint8_t* data) {
  // Set pins for reading
  digitalWrite(NOE, HIGH);
  digitalWrite(NWE, HIGH);
  digitalWrite(NCE, HIGH);
  set_data_pin_mode(INPUT);

  // Assert the address onto the pins, and enable the chip and the output. Wait 100ns for everything to settle
  write_address_pins(address);
  digitalWrite(NCE, LOW);
  digitalWrite(NOE, LOW);
  asm(NOP);
  asm(NOP);

  // Read in the data, and then disable the chip and put its outputs into high impedence state
  *data = read_data_pins();
  digitalWrite(NOE, HIGH);
  digitalWrite(NCE, HIGH);
  set_data_pin_mode(OUTPUT);
  asm(NOP);
  asm(NOP);
}

void write_byte(uint16_t address, uint8_t data) {
  // Prepare pins for a write cycle
  digitalWrite(NOE, HIGH);
  digitalWrite(NWE, HIGH);
  digitalWrite(NCE, HIGH);
  set_data_pin_mode(OUTPUT);
  write_address_pins(address);
  write_data_pins(data);

  // Initiate a write cycle
  // Address is latched on the falling edge of NCE or NWE, whichever is last
  digitalWrite(NCE, LOW);
  digitalWrite(NWE, LOW);
  asm(NOP);
  asm(NOP);

  // Data is latched on the rising edge of NWE or NCE, whichever is first
  digitalWrite(NWE, HIGH);
  digitalWrite(NCE, HIGH);

  // A write cycle takes anywhere between 6ms - 10ms total. 5ms appears to be too short of a delay
  // New data cannot be written until the previous cycle has completed
  delay(6);
}

void read_page(uint16_t page_address, uint8_t* page_buffer) {
  for (uint8_t offset = 0; offset < 64; offset++) {
    uint8_t data = 0;
    read_byte(page_address + offset, &data);
    page_buffer[offset] = data;
  }
}

bool verify_page(uint16_t page_address, uint8_t* page_data, uint8_t chunk_size) {
  for (uint8_t offset = 0; offset < chunk_size; offset++) {
    uint8_t data = 0;
    read_byte(page_address + offset, &data);
    if (data != *(page_data + offset)) {
      return false;
    }
  }
  return true;
}
