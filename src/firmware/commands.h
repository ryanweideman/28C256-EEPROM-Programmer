#ifndef _COMMANDS_H
#define _COMMANDS_H

#include <stdint.h>

#define WRITE_BYTE_COMMAND  0
#define READ_BYTE_COMMAND   1
#define WRITE_CHUNK_COMMAND 2
#define MEMDUMP_COMMAND     3

void write_byte_command(uint16_t address, uint8_t data);
void read_byte_command(uint16_t address);
void write_page_command(uint16_t page_address, uint8_t* page_data);
void read_page_command(uint16_t page_address);
void write_chunk_command(uint16_t chunk_address, uint8_t* chunk_data, uint16_t bytes_in_chunk);
void memdump_command(uint16_t lower_address, uint16_t upper_address);

#endif
