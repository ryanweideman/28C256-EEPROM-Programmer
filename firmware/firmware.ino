#include <Arduino.h>
#include "commands.h"
#include "eeprom_28c256.h"

void init_hardware() {
  init_signal_pins();
  init_address_pins();
  set_data_pin_mode(OUTPUT);
  init_indicator_led();
  led_off();

  Serial.setTimeout(500);
  Serial.begin(115200);
}

void setup() {
  uint8_t command_payload[4096];

  init_hardware();
  led_on();
  Serial.println("ack");

  while (1) {
    if (Serial.available() > 0) {
      uint8_t command = Serial.read();

      if (command == WRITE_CHUNK_COMMAND) {
        Serial.readBytes(command_payload, 4);

        uint16_t chunk_address = ((uint16_t*)command_payload)[0];
        uint16_t chunk_size = ((uint16_t*)command_payload)[1];

        Serial.readBytes(command_payload, chunk_size);
        uint8_t* chunk_data = command_payload;
        write_chunk_command(chunk_address, chunk_data, chunk_size);

      } else if (command == MEMDUMP_COMMAND) {
        Serial.readBytes(command_payload, 4);

        uint16_t lower_address = ((uint16_t*)command_payload)[0];
        uint16_t upper_address = ((uint16_t*)command_payload)[1];

        memdump_command(lower_address, upper_address);

      } else if (command == WRITE_BYTE_COMMAND) {
        Serial.readBytes(command_payload, 3);

        uint16_t address = ((uint16_t*)command_payload)[0];
        uint8_t data = command_payload[2];
        write_byte_command(address, data);

      } else if (command == READ_BYTE_COMMAND) {
        Serial.readBytes(command_payload, 2);

        uint16_t address = ((uint16_t*)command_payload)[0];
        read_byte_command(address);

      } else {
        Serial.println("ERROR: Command with code " + String(command) + " not recognized");
      }
    }
  }
}

/* Implemented to keep the Arduino compiler happy */
void loop() {}
