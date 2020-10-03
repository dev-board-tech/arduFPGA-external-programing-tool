# arduFPGA-external-programing-tool

 A command line tool for writting, reading and debugging arduFPGA microcontroller designs.

### Arguments:

* -s or --log-type: values: "address" or "progress"
* -D or --data: values: memory data to write in hex string format, is mandatory to give the -S, -E, -W and -t parameters.
* -t or --data-type: values: "FLASH", "EEPROM" or "RAM", if -t argument is given then the tool will be able to write, read chucks of memory using -S and -E arguments.
* -W or --write: no value.
* -R or --read: no value.
* -p or --port: value: COM port in string format.
* -b or --baud-rate: value: UART baud rate in string format.
* -S or --start: value: start address in hex format without '0x'.
* -E or --end: value: end address in hex format without '0x'.
* -f or --flash-file: value: FLASH file location and name in string format.
* -e or --eeprom-file: value: EEPROM file location and name in string format.
* -r or --ram-file: value: RAM file location and name in string format.
* -x or --flash-size: value: FLASH size for read purpose, in string hex format without '0x'.
* -y or --eeprom-size: value: EEPROM size for read purpose, in string hex format without '0x'.
* -z or --ram-size: value: RAM size for read purpose, in string hex format without '0x'.


## Usage example:

###### Read and store FLASH memory with size of "0x10000" to "C:\flash_dld.mem" file and EEPROM memory with size of "0x400" to "C:\eeprom_dld.eep" file.

```
ardufpga-ptool.exe --read -p COM10 -b 115200 --log-type progress -x 10000 -f "C:\dld.mem" -y 0400 -e "C:\dld.eep"
```
Response:
```
FLASH: Reading...
READ: <================================> 65536Bytes
EEPROM: Reading...
READ: <================================> 1024Bytes
OK: Reading memory.
```
##### Dump the content of a memory to terminal:

```
ardufpga-ptool.exe --read -p COM10 -b 115200 -t RAM -S 0002 -E 00018
```
Response:
```
>0002:  00 00 00 00 00 00 00 00 00 00 00 00 00 00
>0010: 00 00 00 00 00 00 00 00
OK: Reading RAM memory.
```
