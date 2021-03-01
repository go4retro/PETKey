#---------------- Fuses ----------------

# Set length of binary for bootloader
# WARNING: Fuse settings not tested!

ifeq ($(MCU),atmega128)
  BINARY_LENGTH = 0x1f000
#  EFUSE = 0xff
#  HFUSE = 0x91
#  LFUSE = 0xaf
else ifeq ($(MCU),atmega1281)
  BINARY_LENGTH = 0x1f000
  BOOTLDRSIZE = 0x0800
  EFUSE = 0xff
  HFUSE = 0xd2
  LFUSE = 0xfc
else ifeq ($(MCU),atmega2560)
  BINARY_LENGTH = 0x3f000
  EFUSE = 0xfd
  HFUSE = 0x93
  LFUSE = 0xef
else ifeq ($(MCU),atmega2561)
  BINARY_LENGTH = 0x3f000
  EFUSE = 0xfd
  HFUSE = 0x93
  LFUSE = 0xef
else ifeq ($(MCU),atmega644)
  BINARY_LENGTH = 0xf000
  EFUSE = 0xfd
  HFUSE = 0x91
  LFUSE = 0xef
else ifeq ($(MCU),atmega644p)
  BINARY_LENGTH = 0xf000
  EFUSE = 0xfd
  HFUSE = 0x91
  LFUSE = 0xef
else ifeq ($(MCU),atmega1284p)
  BINARY_LENGTH = 0x1f000
  EFUSE = 0xfd
  HFUSE = 0xd2
  LFUSE = 0xe7
else ifeq ($(MCU),atmega168)
  BINARY_LENGTH = 0x3800
  EFUSE = 0xf9
  HFUSE = 0xdf
  LFUSE = 0xff
else ifeq ($(MCU),atmega328)
#  BINARY_LENGTH = 0x7800
  BINARY_LENGTH = 0x7600
  EFUSE = 0xf9
  HFUSE = 0xdf
  LFUSE = 0xff
else ifeq ($(MCU),atmega328p)
#  BINARY_LENGTH = 0x7800
  BINARY_LENGTH = 0x7600
  EFUSE = 0xf9
  HFUSE = 0xdf
  LFUSE = 0xff
else ifeq ($(MCU),atmega328pb)
#  BINARY_LENGTH = 0x7800
  BINARY_LENGTH = 0x7600
  EFUSE = 0xf9
  HFUSE = 0xdf
  LFUSE = 0xff
else
.PHONY: nochip
nochip:
	@echo '=============================================================='
	@echo 'No known target chip specified.'
	@echo
	@echo 'Please edit the Makefile.'
	@exit 1
endif

