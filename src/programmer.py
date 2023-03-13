import argparse
import math
import serial
import sys
import time
import os

DEFAULT_PORT = '/dev/ttyACM0'

WRITE_BYTE_OPCODE  = 0
READ_BYTE_OPCODE   = 1
WRITE_CHUNK_OPCODE = 2
MEMDUMP_OPCODE     = 3

CHUNK_SIZE  = 4096
EEPROM_SIZE = 32768

BAUD_RATE = 115200

DEFAULT_SERIAL_TIMEOUT = 5

def split_16bit_to_8bit(val):
    return [val & 0xFF, (val >> 8) & 0xFF]

def serial_line_read(port, timeout=DEFAULT_SERIAL_TIMEOUT):
    start_time = time.time()
    while not port.in_waiting:
        if (time.time() - start_time > timeout):
            print("error: serial timeout")
            sys.exit(1)
        continue
    return port.readline().decode('UTF-8').rstrip();

def write_byte_command(port, address, data):
    print("> writing " + str(data) + " at address " + str(address))

    packet = \
        [WRITE_BYTE_OPCODE] + \
        split_16bit_to_8bit(address) + \
        [data]

    port.write(bytearray(packet))

    result = serial_line_read(port)

    if (result == "failure"):
        print(result+'\n')
        return
    print()

    old_byte = serial_line_read(port)
    new_byte = serial_line_read(port)

    print("[{:04x}] {:02x} -> {:02x}".format(int(address), int(old_byte), int(new_byte)))

def read_byte_command(port, address):
    print("> reading byte at address: " + str(address) + "\n")

    packet = \
        [READ_BYTE_OPCODE] + \
        split_16bit_to_8bit(address)

    port.write(bytearray(packet))

    byte = serial_line_read(port)

    print("[{:04x}] {:02x}".format(int(address), int(byte)))

def write_image_command(port, path):
    bin_file = open(path, "r")
    bin_data = [int(b, 16) for b in bin_file.readline().split()]
    bin_file.close()

    start_time = time.time()
    write_data(port, 0, bin_data)
    print("validating data")
    validate_image_command(port, path)

    print("total time elapsed: {:.2f}s".format(time.time() - start_time))

def write_data(port, start_address, bin):
    if (start_address + len(bin) - 1 >= EEPROM_SIZE):
        print("error: attempting to write data at addresses greater than " + str(EEPROM_SIZE-1))
        sys.exit()

    num_chunks = math.ceil(len(bin) / CHUNK_SIZE)
    chunk_count = 0;
    while chunk_count < num_chunks:
        # grab the next chunk of 4096 bytes from the image
        address = start_address + chunk_count * CHUNK_SIZE
        chunk_data = bin[chunk_count * CHUNK_SIZE : min((chunk_count + 1) * CHUNK_SIZE, len(bin))]
        bytes_in_chunk = len(chunk_data)

        packet = \
            [WRITE_CHUNK_OPCODE] + \
            split_16bit_to_8bit(address) + \
            split_16bit_to_8bit(bytes_in_chunk) + \
            chunk_data

        port.write(bytearray(packet))
        result = serial_line_read(port)

        if result == "failed":
            print("\nFAILED")
            break

        if (bytes_in_chunk == 1):
            print("wrote " + \
                str(bytes_in_chunk) + \
                " byte at address " + \
                "[{:04x}] ".format(address))
        else:
            print("wrote " + \
                str(bytes_in_chunk) + \
                " bytes at addresses " + \
                "[{:04x}-{:04x}] ".format(address, address + bytes_in_chunk-1))
        chunk_count += 1
    print()

def validate_image_command(port, path):
    bin_file = open(path, "r")
    bin_data = [int(b, 16) for b in bin_file.readline().split()]
    bin_file.close()

    packet = \
        [MEMDUMP_OPCODE] + \
        split_16bit_to_8bit(0) + \
        split_16bit_to_8bit(len(bin_data) - 1)

    port.write(bytearray(packet))

    failed = False
    address = 0
    for data in bin_data:
        line = serial_line_read(port)
        if data != int(line):
            failed = True
            print("[{:04x}] expected: {:02x} actual: {:02x}".format(address, data, int(line)))
        address += 1
    if failed:
        print("FAILED")
    else:
        print("SUCCESS validated all " + str(len(bin_data)) + " bytes")


def memdump_command(port, lower_address, upper_address):
    print("> reading memory in range [{:04x}-{:04x}]\n".format(lower_address, upper_address))

    packet = \
        [MEMDUMP_OPCODE] + \
        split_16bit_to_8bit(lower_address) + \
        split_16bit_to_8bit(upper_address)

    port.write(bytearray(packet))
    command_completed = False
    current_line = ""
    current_line_len = 0
    address = lower_address
    while address <= upper_address:
        line = serial_line_read(port)
        current_line += "{:02x} ".format(int(line))
        current_line_len += 1

        if current_line_len + address > upper_address:
            current_line = "[{:04x}-{:04x}] ".format(address, address + current_line_len-1) + current_line
            print(current_line)
            break
        elif (current_line_len > 15):
            current_line = "[{:04x}-{:04x}] ".format(address, address + current_line_len-1) + current_line
            print(current_line)
            current_line_len = 0
            current_line = ""
            address += 16

def memset_command(port, lower_address, upper_address, data):
    start_time = time.time()
    bin = [data] * (upper_address - lower_address + 1)
    write_data(port, lower_address, bin)

    print("total time elapsed: {:.2f}s".format(time.time() - start_time))

def clear_command(port):
    start_time = time.time()
    bin = [0] * EEPROM_SIZE
    write_data(port, 0, bin)

    print("total time elapsed: {:.2f}s".format(time.time() - start_time))

def parse_args():
    parser = argparse.ArgumentParser()
    parser.add_argument('command', help="command: memdump, write-image, validate-image, write-byte, read-byte")
    parser.add_argument('--port', '-p', help="serial port", default=DEFAULT_PORT)
    parser.add_argument('--address', '-a', type=int, \
        help="address in range [0, 32767], required for write-byte and read-byte commands")
    parser.add_argument('--lower-address', '-la', type=int, \
        help="Address in range [0, 32767], required for memdump commands", \
        default=0)
    parser.add_argument('--upper-address', '-ua', type=int, \
        help="Address in range [0, 32767], required for memdump commands", \
        default=EEPROM_SIZE-1)
    parser.add_argument('--data', '-d', type=int, \
        help="Data in range [0, 255], required for write-byte command")
    parser.add_argument('--image', '-i', \
        help="32KB bin image, required for write-image command")
    args = parser.parse_args()

    if args.command == 'write-byte':
        if args.address is None or args.data is None:
            parser.error("write-byte requires --address [-a] and --data [-d] arguments")
        if args.address < 0 or args.address >= EEPROM_SIZE:
            parser.error("address must be in range [0, 32767]")
        if args.data < 0 or args.data >= 256:
            parser.error("data must be in range [0, 256)")

    elif args.command == 'read-byte':
        if args.address is None:
            parser.error("read-byte requires the --address [-a] argument")
        if args.address < 0 or args.address >= EEPROM_SIZE:
            parser.error("address must be in range [0, 32767]")

    elif args.command == 'write-image':
        if args.image is None:
            parser.error("write-image requires an image file via the --image [-i] argument")

    elif args.command == 'validate-image':
        if args.image is None:
            parser.error("validate-image requires an image file via the --image [-i] argument")


    elif args.command == 'memdump':
        if args.lower_address < 0 or args.lower_address >= EEPROM_SIZE:
            parser.error("lower-address must be in range [0, 32767]")
        if args.upper_address < 0 or args.upper_address >= EEPROM_SIZE:
            parser.error("upper-address must be in range [0, 32767]")
        if args.lower_address >= args.upper_address:
            parser.error("lower-address must be less than upper_address")

    elif args.command == 'memset':
        if args.data is None:
            parser.error("memset requires --data [-d] arguments")
        if args.data < 0 or args.data >= 256:
            parser.error("data must be in range [0, 256)")
        if args.lower_address < 0 or args.lower_address >= EEPROM_SIZE:
            parser.error("lower-address must be in range [0, 32767]")
        if args.upper_address < 0 or args.upper_address >= EEPROM_SIZE:
            parser.error("upper-address must be in range [0, 32767]")
        if args.lower_address >= args.upper_address:
            parser.error("lower-address must be less than upper_address")

    elif args.command == 'clear':
        pass

    else:
        parser.error("command \'" + args.command + "\' not found")


    return args

def main():
    args = parse_args()

    try:
        os.system('sudo chmod a+rw ' + args.port)
        port = serial.Serial(args.port, BAUD_RATE, timeout=1)
    except:
        print("Could not open port \"" + args.port + "\"")
        sys.exit(1)

    print("> connected to arduino at port " + args.port)
    # waits for programmer to indicate that it is ready for commands
    serial_line_read(port)
    port.flushInput()

    if args.command == 'write-byte':
        write_byte_command(port, args.address, args.data)

    elif args.command == 'read-byte':
        read_byte_command(port, args.address)

    elif args.command == 'write-image':
        write_image_command(port, args.image)

    elif args.command == 'validate-image':
        validate_image_command(port, args.image)

    elif args.command == 'memdump':
        memdump_command(port, args.lower_address, args.upper_address)

    elif args.command == 'memset':
        memset_command(port, args.lower_address, args.upper_address, args.data)

    elif args.command == 'clear':
        clear_command(port)

    port.flushInput()

if __name__ == "__main__":
    main()
