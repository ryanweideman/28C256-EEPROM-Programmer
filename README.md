# 28C256-EEPROM-Programmer
A software and hardware solution for programming [28C256 32KB EEPROMs](https://www.jameco.com/z/28C256-25-Major-Brands-IC-28C256-25IC-EEPROM-256K-Bit-CMOS-Parallel_74878.html). 
- A custom hardware protoboard with a ZIF (zero insertion force) socket for the IC. Based on the Arduino Mega 2560
- Firmware to read and write data to the EEPROM and listen for user provided commands over UART
- A Python CLI tool for writing binary files, analyzing memory, and writing and reading individual bytes in the memory
<br>
<img src="/images/eeprom_with_chip.png" width="50%">

## Demo
#### Writing a File to Memory
The below sample hex file will be programmed to the EEPROM. It contains 32 bytes counting from 0 to 31, specified as hex strings and delimited by spaces.
```Bash
$ cat sample.hex 
00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
```
The ```write-image``` command takes the hex file as input, sends the data to the programmer which writes the 32 bytes to the EEPROM, and reads them back to validate them against the original file.
```Bash
$ python3 programmer.py write-image --image sample.hex
> connected to arduino at port /dev/ttyACM0

wrote 32 bytes at addresses [0000-001f] 

validating data
SUCCESS validated all 32 bytes
total time elapsed: 0.04s
```
Performing a ```memdump``` command with the upper address set to 47 prints the first 48 bytes of the memory. The bytes from the original hex file are confirmed written to addresses [0000-001f]. The other printed 16 bytes remain unprogrammed and equal to zero from the last time the EEPROM was fully reset.
```Bash
$ python3 programmer.py memdump --upper-address 47
> connected to arduino at port /dev/ttyACM0
> reading memory in range [0000-003f]

[0000-000f] 00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 
[0010-001f] 10 11 12 13 14 15 16 17 18 19 1a 1b 1c 1d 1e 1f 
[0020-002f] 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 
```
#### Reading and Writing Individual Addresses
This is an example of the ```write-byte``` command, changing data at address 12345 to 255. It shows that address 0x3039 (12345 decimal) was originally 0x00 and is now changed to 0xFF.
```Bash
$ python3 programmer.py write-byte --address 12345 --data 255
> connected to arduino at port /dev/ttyACM0
> writing 255 at address 12345

[3039] 00 -> ff
```
Similarly, the ```read-byte``` command will read the byte at a given address from the EEPROM
```Bash
$ python3 programmer.py read-byte --address 12345
> connected to arduino at port /dev/ttyACM0
> reading byte at address: 12345

[3039] ff
```

## Commands
## Schematics
