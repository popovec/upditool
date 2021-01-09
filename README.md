# upditool

tool for flashing new Microchip MCU (mega0/AVR series).

*************************************************************************

## Hardware

Supports USB-SERIAL TTL adapter as programmer. Several types of USB
to serial converter can be used, here connection schematics:

      PL2303HX UART
      CP2102/CP2109 UART
      CH340 UART
                                         Microchip UPDI device
          +---------+                   +--------------------+
          |  3.3/5V +-------------------+ Vcc                |
     +----|         |                   |                    |
     | U  |      RX +--------------+----+ UPDI               |
     | S  |         |              |    |                    |
     | B  |         |   +------+   |    |                    |
     +----|      TX +---|  2k2 |---+    |                    |
          |         |   +------+        |                    |
          |    GND  +-------------------+ GND                |
          +---------+                   +--------------------+

*************************************************************************

## Status

AVR128DA(28,32,48,64):

(serial speed 115200 or 230400)

    CHIP ERASE                      OK
    FLASH           read/write      OK
    EEPROM          read/write      OK
    FUSES           read/write      OK
    LOCK            read/write      OK
    USERROW         read/write      OK
    USERROW write on locked device  OK
    SIGNATURE       read            OK
    TEMPSENSE       read            OK
    SERIAL          read            OK

To list all available memory regions use:

    upditool -p avr128da32 -P /dev/null -U help:::

To list supported devices use:

    upditool -p help -P /dev/null

### No TINY0/1 devices support for now
    (due missing specifications in device_avr.h)
    

*************************************************************************

## Examples

(upditool tries to use the same command line switches as avrdude)

### read flash into file flash.bin (raw)

    upditool -U flash:r:flash.bin:r -p avr128da32 -P /dev/ttyUSB2

### read eeprom into file eeprom.hex (intel hex):

    upditool -U eeprom:r:eeprom.bin:i -p avr128da32 -P /dev/ttyUSB2

### write userrow (only 1st two bytes) from command line immediete arguments

    upditool -p avr128da32 -P /dev/ttyUSB2 -U userrow:w:0x55,0xaa:m

### multiple memories verify/read:

    upditool -U signature:v:0x1e,0x97,0x09:m -U serial:r:/dev/null:h
                -U eeprom:r:-:h -U TEMPSENSE0:r:-:h -U TEMPSENSE1:r:-:h
                -U lock:r:-:h -U userrow:r:-:h
                -p avr128da32 -P /dev/ttyUSB2

### lock device

    upditool -p avr128da32 -P /dev/ttyUSB2 -U lock0:w:0x00:m

*************************************************************************

## Compilation

make

copy "upditool" to your favorite directory for local executables


### debian users:

fakeroot debian/rules binary

then install package

### WIN 10

The preferred method is compilation in a Linux environment or you can use WSL.
https://docs.microsoft.com/en-us/windows/wsl/install-win10

Please install packages and run compilation:

	apt install git make mingw-w64
	git clone https://github.com/popovec/upditool
	cd upditool
	make upditool.exe


*************************************************************************
I wrote this software,  because support for AVR DA devices is incomplete in 
https://github.com/mraardvark/pyupdi  (Sep 2020)

Many thanks for inspiration to:
<https://github.com/mraardvark>,
<https://github.com/Polarisru/updiprog>

Special thanks to 
<https://www.avrfreaks.net/forum/which-updi#comment-2391851>
(BOOTDONE, OCD KEY etc..)
