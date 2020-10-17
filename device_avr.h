/*
    device_avr.h

    This is part of c_updi, programmer

    Copyright (C) 2020 Peter Popovec, popovec.peter@gmail.com

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.


    device database
*/

#define MEM_AVR128DA  \
{ .start =   0x1100, .size = 3,  .psize = 3,                    .rw = 0, .nvm = 1, .name = "signature",},\
{ .start =   0x1110, .size = 16, .psize = 16,                   .rw = 0, .nvm = 1, .name = "serial",},\
{ .start =   0x1104, .size = 2,  .psize = 2,                    .rw = 0, .nvm = 1, .name = "TEMPSENSE0",},\
{ .start =   0x1106, .size = 2,  .psize = 2,                    .rw = 0, .nvm = 1, .name = "TEMPSENSE1",},\
/* EEPROM allow us to erase 32 bytes in one operation, we use this as pagesize */\
{ .start =   0x1400, .size = 512,.psize = 32,.wstart = 0x1400, .rw = 1, .nvm = 1, .name = "eeprom",   .write_algo=C_WRITE_ALGO_NVM_EEPROM_D},\
{ .start =   0x1080, .size = 32, .psize = 32, .wstart = 0x4000, .rw = 1, .nvm = 1, .name = "userrow",  .write_algo=C_WRITE_ALGO_USERROW_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1080, .size = 32, .psize = 32, .wstart = 0x4000, .rw = 1, .nvm = 1, .name = "usersig",  .write_algo=C_WRITE_ALGO_USERROW_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1050, .size = 1,  .psize = 1,  .wstart = 0x1050, .rw = 1, .nvm = 1, .name = "fuse0",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1050, .size = 8,  .psize = 1,  .wstart = 0x1050, .rw = 1, .nvm = 1, .name = "fuses",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1050, .size = 1,  .psize = 1,  .wstart = 0x1050, .rw = 1, .nvm = 1, .name = "WDTCFG",   .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1051, .size = 1,  .psize = 1,  .wstart = 0x1051, .rw = 1, .nvm = 1, .name = "fuse1",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1051, .size = 1,  .psize = 1,  .wstart = 0x1051, .rw = 1, .nvm = 1, .name = "BODCFG",   .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1052, .size = 1,  .psize = 1,  .wstart = 0x1052, .rw = 1, .nvm = 1, .name = "fuse2",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1052, .size = 1,  .psize = 1,  .wstart = 0x1052, .rw = 1, .nvm = 1, .name = "OSCCFG",   .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1055, .size = 1,  .psize = 1,  .wstart = 0x1055, .rw = 1, .nvm = 1, .name = "fuse5",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1055, .size = 1,  .psize = 1,  .wstart = 0x1055, .rw = 1, .nvm = 1, .name = "SYSCFG0",  .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1056, .size = 1,  .psize = 1,  .wstart = 0x1056, .rw = 1, .nvm = 1, .name = "fuse6",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1056, .size = 1,  .psize = 1,  .wstart = 0x1056, .rw = 1, .nvm = 1, .name = "SYSCFG1",  .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1057, .size = 1,  .psize = 1,  .wstart = 0x1057, .rw = 1, .nvm = 1, .name = "fuse7",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1057, .size = 1,  .psize = 1,  .wstart = 0x1057, .rw = 1, .nvm = 1, .name = "CODESIZE", .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1058, .size = 1,  .psize = 1,  .wstart = 0x1058, .rw = 1, .nvm = 1, .name = "fuse8",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1058, .size = 1,  .psize = 1,  .wstart = 0x1058, .rw = 1, .nvm = 1, .name = "BOOTSIZE", .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1040, .size = 4,  .psize = 4,  .wstart = 0x1040, .rw = 1, .nvm = 1, .name = "lock",     .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1040, .size = 1,  .psize = 1,  .wstart = 0x1040, .rw = 1, .nvm = 1, .name = "lock0",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1041, .size = 1,  .psize = 1,  .wstart = 0x1041, .rw = 1, .nvm = 1, .name = "lock1",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1042, .size = 1,  .psize = 1,  .wstart = 0x1042, .rw = 1, .nvm = 1, .name = "lock2",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start =   0x1043, .size = 1,  .psize = 1,  .wstart = 0x1043, .rw = 1, .nvm = 1, .name = "lock3",    .write_algo=C_WRITE_ALGO_NVM_EEPROM_D, .not_affected_by_chip_erase = 1},\
{ .start = 0x800000, .size = 0x20000, .psize = 512, .wstart = 0x800000, \
                                                                .rw = 1, .nvm = 1, .name = "flash",    .write_algo=C_WRITE_ALGO_FLASH_D},\
{ .start =   0x4000, .size = 16384, .psize = 16384, .wstart = 0x4000,  .rw = 1, .nvm = 0, .name = "ram",},\
{ .name = "",},


    {
    .sib = "    AVR P:2D:1-3",
    .name = "avr128da28",
    .signature[0] = 0x1e,
    .signature[1] = 0x97,
    .signature[2] = 0x0A,
    .ptr_bytes = 3,
    .if_locked_ignore_signature_test = 1,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_D,
    .memory = { MEM_AVR128DA },
    },
    {
    .sib = "    AVR P:2D:1-3",
    .name = "avr128da32",
    .signature[0] = 0x1e,
    .signature[1] = 0x97,
    .signature[2] = 0x09,
    .ptr_bytes = 3,
    .if_locked_ignore_signature_test = 1,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_D,
    .memory = { MEM_AVR128DA },
    },
    {
    .sib = "    AVR P:2D:1-3",
    .name = "avr128da48",
    .signature[0] = 0x1e,
    .signature[1] = 0x97,
    .signature[2] = 0x08,
    .ptr_bytes = 3,
    .if_locked_ignore_signature_test = 1,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_D,
    .memory = { MEM_AVR128DA },
    },
    {
    .sib = "    AVR P:2D:1-3",
    .name = "avr128da64",
    .signature[0] = 0x1e,
    .signature[1] = 0x97,
    .signature[2] = 0x07,
    .ptr_bytes = 3,
    .if_locked_ignore_signature_test = 1,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_D,
    .memory = { MEM_AVR128DA },
    },
#undef MEM_AVR128DA


#define MEM_megaAVR480X \
{ .start =  0x1100, .size = 3,                        .rw = 0, .nvm = 1, .name = "signature",},\
{ .start =  0x1103, .size = 10,                       .rw = 0, .nvm = 1, .name = "serial",},\
{ .start =  0x1118, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL16M0",},\
{ .start =  0x1119, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL16M1",},\
{ .start =  0x111A, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL20M0",},\
{ .start =  0x111B, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL20M1",},\
{ .start =  0x1120, .size = 1,                        .rw = 0, .nvm = 1, .name = "TEMPSENSE0",},\
{ .start =  0x1121, .size = 1,                        .rw = 0, .nvm = 1, .name = "TEMPSENSE1",},\
{ .start =  0x1122, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC16ERR3V",},\
{ .start =  0x1123, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC16ERR5V",},\
{ .start =  0x1124, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC20ERR3V",},\
{ .start =  0x1125, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC20ERR5V",},\
{ .start =  0x1400, .size = 256,.psize = 64,     .wstart = 0x1400, .rw = 1, .nvm = 1, .name = "eeprom",  .write_algo=C_WRITE_ALGO_NVMmega0,},\
{ .start =  0x4000, .size = 0xc000, .psize =128, .wstart = 0x4000, .rw = 1, .nvm = 1, .name = "flash",   .write_algo=C_WRITE_ALGO_NVMmega0,},\
/* mega4808/4809 - userrow programming in chapter 30.3.7.3 is not used here, NVM controller can program this area.. please read chapter 9.3.1.3 */ \
/* for now, this can not be done on locked device, if device is locked, algorhithm from chapter 30.3.7.3 should be used */ \
{ .start =  0x1300, .size = 64, .psize = 64,     .wstart = 0x1300, .rw = 1, .nvm = 1, .name = "userrow", .write_algo=C_WRITE_ALGO_NVMmega0, .not_affected_by_chip_erase = 1 },\
{ .start =  0x1300, .size = 64, .psize = 64,     .wstart = 0x1300, .rw = 1, .nvm = 1, .name = "usersig", .write_algo=C_WRITE_ALGO_NVMmega0, .not_affected_by_chip_erase = 1 },\
{ .start =  0x1280, .size = 1,  .psize = 1,      .wstart = 0x1280, .rw = 1, .nvm = 1, .name = "fuse0",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1280, .size = 1,  .psize = 1,      .wstart = 0x1280, .rw = 1, .nvm = 1, .name = "WDTCFG",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1281, .size = 1,  .psize = 1,      .wstart = 0x1281, .rw = 1, .nvm = 1, .name = "fuse1",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1281, .size = 1,  .psize = 1,      .wstart = 0x1281, .rw = 1, .nvm = 1, .name = "BODCFG",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1282, .size = 1,  .psize = 1,      .wstart = 0x1282, .rw = 1, .nvm = 1, .name = "fuse2",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1282, .size = 1,  .psize = 1,      .wstart = 0x1282, .rw = 1, .nvm = 1, .name = "OSCCFG",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1285, .size = 1,  .psize = 1,      .wstart = 0x1285, .rw = 1, .nvm = 1, .name = "fuse5",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1285, .size = 1,  .psize = 1,      .wstart = 0x1285, .rw = 1, .nvm = 1, .name = "SYSCFG0", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1286, .size = 1,  .psize = 1,      .wstart = 0x1286, .rw = 1, .nvm = 1, .name = "fuse6",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1286, .size = 1,  .psize = 1,      .wstart = 0x1286, .rw = 1, .nvm = 1, .name = "SYSCFG1", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1287, .size = 1,  .psize = 1,      .wstart = 0x1287, .rw = 1, .nvm = 1, .name = "fuse7",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1287, .size = 1,  .psize = 1,      .wstart = 0x1287, .rw = 1, .nvm = 1, .name = "APPEND",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1288, .size = 1,  .psize = 1,      .wstart = 0x1288, .rw = 1, .nvm = 1, .name = "fuse8",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1288, .size = 1,  .psize = 1,      .wstart = 0x1288, .rw = 1, .nvm = 1, .name = "BOOTEND", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "fuse10",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "lock",    .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "lock0",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "LOCKBIT", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .name = "",},


    {
    .sib = "megaAVR P:0D:1-3",
    .name = "mega4808",
    .signature[0] = 0x1e,
    .signature[1] = 0x96,
    .signature[2] = 0x50,
    .ptr_bytes = 2,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_mega0,
    .memory = { MEM_megaAVR480X },
    },
    {
    .sib = "megaAVR P:0D:1-3",
    .name = "mega4809",
    .signature[0] = 0x1e,
    .signature[1] = 0x96,
    .signature[2] = 0x51,
    .ptr_bytes = 2,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_mega0,
    .memory = { MEM_megaAVR480X },
    },

#undef MEM_megaAVR480X
#define MEM_megaAVR320X \
{ .start =  0x1100, .size = 3,                        .rw = 0, .nvm = 1, .name = "signature",},\
{ .start =  0x1103, .size = 10,                       .rw = 0, .nvm = 1, .name = "serial",},\
{ .start =  0x1118, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL16M0",},\
{ .start =  0x1119, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL16M1",},\
{ .start =  0x111A, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL20M0",},\
{ .start =  0x111B, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSCCAL20M1",},\
{ .start =  0x1120, .size = 1,                        .rw = 0, .nvm = 1, .name = "TEMPSENSE0",},\
{ .start =  0x1121, .size = 1,                        .rw = 0, .nvm = 1, .name = "TEMPSENSE1",},\
{ .start =  0x1122, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC16ERR3V",},\
{ .start =  0x1123, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC16ERR5V",},\
{ .start =  0x1124, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC20ERR3V",},\
{ .start =  0x1125, .size = 1,                        .rw = 0, .nvm = 1, .name = "OSC20ERR5V",},\
{ .start =  0x1400, .size = 256,.psize = 64,     .wstart = 0x1400, .rw = 1, .nvm = 1, .name = "eeprom",  .write_algo=C_WRITE_ALGO_NVMmega0,},\
{ .start =  0x4000, .size = 0x8000, .psize =128, .wstart = 0x4000, .rw = 1, .nvm = 1, .name = "flash",   .write_algo=C_WRITE_ALGO_NVMmega0,},\
{ .start =  0x1300, .size = 64, .psize = 64,     .wstart = 0x1300, .rw = 1, .nvm = 1, .name = "userrow", .write_algo=C_WRITE_ALGO_NVMmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1300, .size = 64, .psize = 64,     .wstart = 0x1300, .rw = 1, .nvm = 1, .name = "usersig", .write_algo=C_WRITE_ALGO_NVMmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1280, .size = 1,  .psize = 1,      .wstart = 0x1280, .rw = 1, .nvm = 1, .name = "fuse0",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1280, .size = 1,  .psize = 1,      .wstart = 0x1280, .rw = 1, .nvm = 1, .name = "WDTCFG",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1281, .size = 1,  .psize = 1,      .wstart = 0x1281, .rw = 1, .nvm = 1, .name = "fuse1",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1281, .size = 1,  .psize = 1,      .wstart = 0x1281, .rw = 1, .nvm = 1, .name = "BODCFG",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1282, .size = 1,  .psize = 1,      .wstart = 0x1282, .rw = 1, .nvm = 1, .name = "fuse2",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1282, .size = 1,  .psize = 1,      .wstart = 0x1282, .rw = 1, .nvm = 1, .name = "OSCCFG",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1285, .size = 1,  .psize = 1,      .wstart = 0x1285, .rw = 1, .nvm = 1, .name = "fuse5",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1285, .size = 1,  .psize = 1,      .wstart = 0x1285, .rw = 1, .nvm = 1, .name = "SYSCFG0", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1286, .size = 1,  .psize = 1,      .wstart = 0x1286, .rw = 1, .nvm = 1, .name = "fuse6",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1286, .size = 1,  .psize = 1,      .wstart = 0x1286, .rw = 1, .nvm = 1, .name = "SYSCFG1", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1287, .size = 1,  .psize = 1,      .wstart = 0x1287, .rw = 1, .nvm = 1, .name = "fuse7",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1287, .size = 1,  .psize = 1,      .wstart = 0x1287, .rw = 1, .nvm = 1, .name = "APPEND",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1288, .size = 1,  .psize = 1,      .wstart = 0x1288, .rw = 1, .nvm = 1, .name = "fuse8",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x1288, .size = 1,  .psize = 1,      .wstart = 0x1288, .rw = 1, .nvm = 1, .name = "BOOTEND", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "fuse10",  .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "lock",    .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "lock0",   .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .start =  0x128A, .size = 1,  .psize = 1,      .wstart = 0x128A, .rw = 1, .nvm = 1, .name = "LOCKBIT", .write_algo=C_WRITE_ALGO_FUSEmega0, .not_affected_by_chip_erase = 1},\
{ .name = "",},


    {
    .sib = "megaAVR P:0D:1-3",
    .name = "mega3208",
    .signature[0] = 0x1e,
    .signature[1] = 0x95,
    .signature[2] = 0x30,
    .ptr_bytes = 2,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_mega0,
    .memory = { MEM_megaAVR320X },
    },
    {
    .sib = "megaAVR P:0D:1-3",
    .name = "mega3209",
    .signature[0] = 0x1e,
    .signature[1] = 0x95,
    .signature[2] = 0x31,
    .ptr_bytes = 2,
    .chip_erase_algo = C_ALGO_CHIP_ERASE_mega0,
    .memory = { MEM_megaAVR320X },
    },

#undef MEM_megaAVR320X
