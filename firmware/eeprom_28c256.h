#ifndef _28C256_H
#define _28C256_H

#include <stdint.h>
#include <stdbool.h>

// Address Pin Mappings
#define ADDR_0  34
#define ADDR_1  36
#define ADDR_2  38
#define ADDR_3  40
#define ADDR_4  42
#define ADDR_5  44
#define ADDR_6  46
#define ADDR_7  48
#define ADDR_8  14
#define ADDR_9  15
#define ADDR_10 18
#define ADDR_11 16
#define ADDR_12 50
#define ADDR_13 5
#define ADDR_14 52

// Data Pin Mappings
#define DATA_0  28
#define DATA_1  30
#define DATA_2  32
#define DATA_3  26
#define DATA_4  24
#define DATA_5  22
#define DATA_6  21
#define DATA_7  20

// Signals
#define NCE 19
#define NOE 17
#define NWE 6

#define GREEN_LED 9

#define NOP "nop\n"

void init_address_pins();
void set_data_pin_mode(int mode);
void init_signal_pins();
void init_indicator_led();

void led_on();
void led_off();

void write_led(int state);
void write_address_pins(uint16_t address);
void write_data_pins(uint8_t data);
uint8_t read_data_pins();

void write_byte(uint16_t address, uint8_t data);
void read_byte(uint16_t address, uint8_t* data);

void write_page(uint16_t page_address, uint8_t* page_data, uint8_t page_size);
void read_page(uint16_t page_address, uint8_t* page_buffer);
bool verify_page(uint16_t page_address, uint8_t* page_data, uint8_t page_size);

bool write_chunk(uint16_t chunk_address, uint8_t* chunk_data, uint16_t chunk_size);

#endif
