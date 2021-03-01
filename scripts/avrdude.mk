#---------------- Programming Options (avrdude) ----------------

# Programming hardware: alf avr910 avrisp bascom bsd
# dt006 pavr picoweb pony-stk200 sp12 stk200 stk500 stk500v2
#
# Type: avrdude -c ?
# to get a full listing.
#
#AVRDUDE_PROGRAMMER = stk200
ifdef CONFIG_AVRDUDE_PROGRAMMER
  AVRDUDE_PROGRAMMER := -c $(CONFIG_AVRDUDE_PROGRAMMER)
endif

# com1 = serial port. Use lpt1 to connect to parallel port.
#AVRDUDE_PORT = lpt1    # programmer connected to serial device
ifdef CONFIG_AVRDUDE_PORT
  AVRDUDE_PORT := -P $(CONFIG_AVRDUDE_PORT)
endif

ifdef CONFIG_AVRDUDE_FLAGS
  AVRDUDE_FLAGS := $(CONFIG_AVRDUDE_FLAGS)
else
  AVRDUDE_FLAGS := 
endif

AVRDUDE_WRITE_FLASH = -U flash:w:$(TARGET).hex
# AVRDUDE_WRITE_EEPROM = -U eeprom:w:$(TARGET).eep

# Allow fuse overrides from the config file
ifdef CONFIG_EFUSE
  EFUSE := $(CONFIG_EFUSE)
endif
ifdef CONFIG_HFUSE
  HFUSE := $(CONFIG_HFUSE)
endif
ifdef CONFIG_LFUSE
  LFUSE := $(CONFIG_LFUSE)
endif

# Calculate command line arguments for fuses
AVRDUDE_WRITE_FUSES :=
ifdef EFUSE
  AVRDUDE_WRITE_FUSES += -U efuse:w:$(EFUSE):m
endif
ifdef HFUSE
  AVRDUDE_WRITE_FUSES += -U hfuse:w:$(HFUSE):m
endif
ifdef LFUSE
  AVRDUDE_WRITE_FUSES += -U lfuse:w:$(LFUSE):m
endif


# Uncomment the following if you want avrdude's erase cycle counter.
# Note that this counter needs to be initialized first using -Yn,
# see avrdude manual.
#AVRDUDE_ERASE_COUNTER = -y

# Uncomment the following if you do /not/ wish a verification to be
# performed after programming the device.
#AVRDUDE_NO_VERIFY = -V

# Increase verbosity level.  Please use this when submitting bug
# reports about avrdude. See <http://savannah.nongnu.org/projects/avrdude>
# to submit bug reports.
#AVRDUDE_VERBOSE = -v -v

AVRDUDE_FLAGS += -p $(MCU)
AVRDUDE_FLAGS += $(AVRDUDE_NO_VERIFY)
AVRDUDE_FLAGS += $(AVRDUDE_VERBOSE)
AVRDUDE_FLAGS += $(AVRDUDE_ERASE_COUNTER)
AVRDUDE_FLAGS += $(AVRDUDE_PROGRAMMER) $(AVRDUDE_PORT)

#---------------- Debugging Options ----------------

# For simulavr only - target MCU frequency.
DEBUG_MFREQ = $(CONFIG_MCU_FREQ)

# Set the DEBUG_UI to either gdb or insight.
# DEBUG_UI = gdb
DEBUG_UI = insight

# Set the debugging back-end to either avarice, simulavr.
DEBUG_BACKEND = avarice
#DEBUG_BACKEND = simulavr

# GDB Init Filename.
GDBINIT_FILE = __avr_gdbinit

# When using avarice settings for the JTAG
JTAG_DEV = /dev/com1

# Debugging port used to communicate between GDB / avarice / simulavr.
DEBUG_PORT = 4242

# Debugging host used to communicate between GDB / avarice / simulavr, normally
#     just set to localhost unless doing some sort of crazy debugging when
#     avarice is running on a different computer.
DEBUG_HOST = localhost

