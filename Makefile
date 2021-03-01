# Hey Emacs, this is a -*- makefile -*-

#----------------------------------------------------------------------------
# WinAVR Makefile Template written by Eric B. Weddington, Jörg Wunsch, et al.
#
# Released to the Public Domain
#
# Additional material for this makefile was written by:
# Peter Fleury
# Tim Henigan
# Colin O'Flynn
# Reiner Patommel
# Markus Pfaff
# Sander Pool
# Frederik Rouleau
# Carlos Lamas
#
# Extensively modified for sd2iec by Ingo Korb
# Modified for general purpose usage by Jim Brain
#----------------------------------------------------------------------------
# On command line:
#
# make all = Make software.
#
# make clean = Clean out built project files.
#
# make program = Download the hex file to the device, using avrdude.
#                Please customize the avrdude settings below first!
#
# make filename.s = Just compile filename.c into the assembler code only.
#
# To rebuild project do "make clean all".
#----------------------------------------------------------------------------

# Read configuration file
ifdef CONFIG
 CONFIGSUFFIX = $(CONFIG:config%=%)
else
 CONFIG = config
 CONFIGSUFFIX =
endif

# Include the configuration file
include $(CONFIG)

MCU    := $(CONFIG_MCU)

# Presume the C and asm source files to be located in the subdirectory 'src'.
# Set SRCDIR to override.
ifeq ($(SRCDIR),)
  SRCDIR := src
endif

# Directory for all generated files
OBJDIR := obj-$(MCU:atmega%=m%)$(CONFIGSUFFIX)

# Target file name (without extension).
#TARGET = $(OBJDIR)/PETKey

# Presume we're in a project directory so name the program like the current
# directory. Set TARGET to override.
ifeq ($(TARGET),)
  TARGET := $(OBJDIR)/$(notdir $(CURDIR))
endif
 
# Output format. (can be srec, ihex, binary)
HEXFORMAT = ihex

# List C source files here. (C dependencies are automatically generated.)
SRC  = uart.c 
SRC += main.c
SRC += eeprom.c
SRC += kb.c
SRC += vkb_pet.c
SRC += debug.c

ifeq ($(CONFIG_UART_DEBUG),y)
  SRC += uart.c
endif

ifeq ($(CONFIG_UART_DEBUG_SW),y)
  SRC += swuart.c
endif

# Sample mechanism to add files to SRC line
#ifeq ($(CONFIG_VARIABLE),4)
#  SRC += file.c
#else
#  SRC += file2.c
#endif

# List Assembler source files here.
#     Make them always end in a capital .S.  Files ending in a lowercase .s
#     will not be considered source files but generated files (assembler
#     output from the compiler), and will be deleted upon "make clean"!
#     Even though the DOS/Win* filesystem matches both .s and .S the same,
#     it will preserve the spelling of the filenames, and gcc itself does
#     care about how the name is spelled on its command-line.
ASMSRC =

# Optimization level, can be [0, 1, 2, 3, s].
#     0 = turn off optimization. s = optimize for size.
#     (Note: 3 is not always the best optimization level. See avr-libc FAQ.)
# Use s -mcall-prologues when you really need size...
#OPT = 2
OPT = s

# Debugging format.
#     Native formats for AVR-GCC's -g are dwarf-2 [default] or stabs.
#     AVR Studio 4.10 requires dwarf-2.
#     AVR [Extended] COFF format requires stabs, plus an avr-objcopy run.
DEBUG = dwarf-2

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRAINCDIRS =

# Compiler flag to set the C Standard level.
#     c89   = "ANSI" C
#     gnu89 = c89 plus GCC extensions
#     c99   = ISO C99 standard (not yet fully implemented)
#     gnu99 = c99 plus GCC extensions
CSTANDARD = -std=gnu99

# Place -D or -U options here
CDEFS = -DF_CPU=$(CONFIG_MCU_FREQ)UL

# Place -I options here
CINCS =

#============================================================================

# Define version number
include scripts/gmtt.mk

# create a 3-column table from the header file. The first column is just the "#define"
VER_TABLE := 3 $(file < src/version.h)

MAJOR = $(call select,3,$(VER_TABLE),$$(call str-eq,$$2,VER_MAJOR))
MINOR = $(call select,3,$(VER_TABLE),$$(call str-eq,$$2,VER_MINOR))
PATCHLEVEL = $(call select,3,$(VER_TABLE),$$(call str-eq,$$2,VER_PATCH))
FIX = $(call select,3,$(VER_TABLE),$$(call str-eq,$$2,VER_FIX))

# Forces bootloader version to 0, comment out or leave empty for release
#PRERELEASE = 
PRERELEASE = $(call select,3,$(VER_TABLE),$$(call str-eq,$$2,VER_PRERELEASE))

# Calculate bootloader version
ifdef PRERELEASE
BOOT_VERSION := 0
else
BOOT_VERSION := 0x$(MAJOR)$(MINOR)$(PATCHLEVEL)$(FIX)
endif

# Create a version number define
ifdef PATCHLEVEL
ifdef FIX
PROGRAMVERSION := $(MAJOR).$(MINOR).$(PATCHLEVEL).$(FIX)
else
PROGRAMVERSION := $(MAJOR).$(MINOR).$(PATCHLEVEL)
BOOT_VERSION := $(BOOT_VERSION)0
endif
else
PROGRAMVERSION := $(MAJOR).$(MINOR)
BOOT_VERSION := $(BOOT_VERSION)00
endif

ifdef PRERELEASE
PROGRAMVERSION := $(PROGRAMVERSION)$(PRERELEASE)
endif

LONGVERSION := -$(CONFIG_MCU:atmega%=m%)$(CONFIGSUFFIX)
#CDEFS += -DVERSION=\"$(PROGRAMVERSION)\" -DLONGVERSION=\"$(LONGVERSION)\"
#if version.h is included in code
CDEFS += -DLONGVERSION=\"$(LONGVERSION)\"

#============================================================================

# Define programs and commands.
SHELL = sh
REMOVE = rm -f
COPY = cp
WINSHELL = cmd
AWK = gawk
#---------------- Toolchain ----------------
CC = avr-gcc
CPP = avr-g++
OBJCOPY = avr-objcopy
OBJDUMP = avr-objdump
SIZE = avr-size
NM = avr-nm
AVRDUDE = avrdude
#CRCGEN = crcgen-new
CRCGEN = scripts/crcgen-avr.pl
CONF2H = scripts/conf2h.awk

# Include fuse settings
include scripts/fuses.mk
# Include avrdude settings
include scripts/avrdude.mk

#---------------- Compiler Options ----------------
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -mmcu=$(MCU)
CFLAGS += -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT) 
CFLAGS += -fno-strict-aliasing
CFLAGS += -Wall 
CFLAGS += -Wsign-compare
CFLAGS += -Wunused-parameter
CFLAGS += -Wstrict-prototypes 
# Add this if you want all warnings output as errors.
#CFLAGS += -Werror
CFLAGS += -Wa,-adhlns=$(OBJDIR)/$(<:.c=.lst)
CFLAGS += -I$(OBJDIR) -Isrc
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)
CFLAGS += -ffunction-sections
CFLAGS += -fdata-sections

CFLAGS += -funsigned-char
CFLAGS += -funsigned-bitfields
CFLAGS += -fpack-struct
CFLAGS += -fshort-enums
#CFLAGS += -fno-unit-at-a-time
CFLAGS += -Wundef
CFLAGS += -Wextra
CFLAGS += -Wunreachable-code
CFLAGS += -Wshadow
#CFLAGS += -Winline
#CFLAGS += -mtiny-stack
#CFLAGS += -mno-interrupts
CFLAGS += -mcall-prologues
CFLAGS += -ffreestanding
CFLAGS += -fno-tree-scev-cprop
CFLAGS += -fno-optimize-sibling-calls
CFLAGS += -fno-tree-switch-conversion
#CFLAGS += -maccumulate-args
#CFLAGS += -mstrict-X
CFLAGS += -flto
CFLAGS += -fno-inline-small-functions
#CFLAGS += -finline-limit=3 
#CFLAGS += --param inline-call-cost=3
CFLAGS += -fno-move-loop-invariants
CFLAGS += -fno-split-wide-types

# turn these on to keep the functions in the same order as in the source
# this is only useful if you're looking at disassembly
#CFLAGS += -fno-reorder-blocks
#CFLAGS += -fno-reorder-blocks-and-partition
#CFLAGS += -fno-reorder-functions
#CFLAGS += -fno-toplevel-reorder
#CFLAGS += -fno-tree-loop-optimize

#---------------- Config ----------------
ifeq ($(CONFIG_STACK_TRACKING),y)
  CFLAGS += -finstrument-functions
endif

#---------------- Assembler Options ----------------
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
#  -listing-cont-lines: Sets the maximum number of continuation lines of hex 
#       dump that will be displayed for a given single line of source input.
ASFLAGS  = -mmcu=$(MCU) 
ASFLAGS += -Wa,-adhlns=$(OBJDIR)/$(*F).lst,-gstabs,--listing-cont-lines=100
ASFLAGS += -x assembler-with-cpp 
ASFLAGS += $(CDEFS)

#---------------- Library Options ----------------
# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt

# If this is left blank, then it will use the Standard printf version.
PRINTF_LIB =
#PRINTF_LIB = $(PRINTF_LIB_MIN)
#PRINTF_LIB = $(PRINTF_LIB_FLOAT)

# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min

# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt

# If this is left blank, then it will use the Standard scanf version.
SCANF_LIB =
#SCANF_LIB = $(SCANF_LIB_MIN)
#SCANF_LIB = $(SCANF_LIB_FLOAT)

MATH_LIB = -lm

# List any extra directories to look for libraries here.
#     Each directory must be seperated by a space.
#     Use forward slashes for directory separators.
#     For a directory that has spaces, enclose it in quotes.
EXTRALIBDIRS = 

#---------------- External Memory Options ----------------

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# used for variables (.data/.bss) and heap (malloc()).
#EXTMEMOPTS = -Wl,-Tdata=0x801100,--defsym=__heap_end=0x80ffff

# 64 KB of external RAM, starting after internal RAM (ATmega128!),
# only used for heap (malloc()).
#EXTMEMOPTS = -Wl,--defsym=__heap_start=0x801100,--defsym=__heap_end=0x80ffff
EXTMEMOPTS =

#---------------- Linker Options ----------------
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS  = -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += $(EXTMEMOPTS)
LDFLAGS += $(patsubst %,-L%,$(EXTRALIBDIRS))
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -flto
#LDFLAGS += -Wl,--section-start=.text=$(BOOTLOADERSTARTADR)
#LDFLAGS += -T linker_script.x
ifeq ($(CONFIG_LINKER_RELAX),y)
  LDFLAGS += -Wl,-O1,--relax
endif

#============================================================================

# De-dupe the list of C source files
CSRC := $(patsubst %,$(SRCDIR)/%,$(sort $(SRC)))

# Add subdir to assembler source files
ASMSRC_DIR := $(patsubst %,$(SRCDIR)/%,$(ASMSRC))

# Define all object files.
OBJ := $(patsubst %,$(OBJDIR)/%,$(CSRC:.c=.o) $(ASMSRC_DIR:.S=.o) $(CSRC_DIR:.cpp=.o))

# Define all listing files.
LST := $(patsubst %,$(OBJDIR)/%,$(CSRC:.c=.lst) $(ASMSRC_DIR:.S=.lst))

# Define the object directories
OBJDIRS := $(sort $(dir $(OBJ)))

# Compiler flags to generate dependency files.
GENDEPFLAGS = -MMD -MP -MF .dep/$(@F).d

CFLAGS  += $(GENDEPFLAGS)

# Display size of file.
#HEXSIZE = $(SIZE) --mcu=$(MCU) --target=$(HEXFORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A $(TARGET).elf

# Enable verbose compilation with "make V=1"
ifdef V
 Q :=
 E := @:
else
 Q := @
 E := @echo
endif

#============================================================================

# Default target.
all: build

build: elf hex bin
#build: elf hex bin eep lss 
	$(E) "  SIZE   $(TARGET).elf"
	$(Q)$(ELFSIZE)|grep -v debug

elf: $(TARGET).elf
bin: $(TARGET).bin
hex: $(TARGET).hex
eep: $(TARGET).eep
lss: $(TARGET).lss
sym: $(TARGET).sym

# Program the device.
program: bin hex eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)  $(AVRDUDE_WRITE_EEPROM)

# Set fuses of the device
fuses: $(CONFIG)
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FUSES)

progall: bin hex eep
	$(AVRDUDE) $(AVRDUDE_FLAGS) $(AVRDUDE_WRITE_FLASH)  $(AVRDUDE_WRITE_EEPROM) $(AVRDUDE_WRITE_FUSES)

# Generate autoconf.h from config
.PRECIOUS : $(OBJDIR) $(OBJDIR)/autoconf.h
$(OBJDIR)/autoconf.h: $(CONFIG) | $(OBJDIR)
	$(E) "  CONF2H $(CONFIG)"
	$(Q)$(AWK) -f $(CONF2H) $(CONFIG) > $(OBJDIR)/autoconf.h

# Generate macro-only asmconfig.h from autoconf.h
.PRECIOUS: $(OBJDIR)/asmconfig.h
$(OBJDIR)/asmconfig.h: $(CONFFILES) $(SRCDIR)/config.h | $(OBJDIR)
	$(E) "  CPP    config.h"
	$(Q)$(CC) -E -dM $(ASFLAGS) src/config.h | grep -v "^#define __" > $@

# Create final output files (.hex, .eep) from ELF output file.
ifeq ($(CONFIG_BOOTLOADER),y)
$(OBJDIR)/%.bin: $(OBJDIR)/%.elf
	$(E) "  BIN    $@"
	$(Q)$(OBJCOPY) -O binary -R .eeprom $< $@
	$(E) "  CRCGEN $@"
	-$(Q)$(CRCGEN) $@ $(BINARY_LENGTH) $(CONFIG_BOOT_DEVID) $(BOOT_VERSION)
	$(E) "  COPY   $(CONFIG_HARDWARE_NAME)-firmware-$(PROGRAMVERSION).bin"
	$(Q)$(COPY) $@ $(OBJDIR)/$(CONFIG_HARDWARE_NAME)-firmware-$(PROGRAMVERSION).bin
else
$(OBJDIR)/%.bin: $(OBJDIR)/%.elf
	$(E) "  BIN    $@"
	$(Q)$(OBJCOPY) -O binary -R .eeprom $< $@
endif

# create hex program file from ELF output file
$(OBJDIR)/%.hex: $(OBJDIR)/%.elf
	$(E) "  HEX    $@"
	$(Q)$(OBJCOPY) -O $(HEXFORMAT) -R .eeprom $< $@

# create eeprom data file from ELF output file
$(OBJDIR)/%.eep: $(OBJDIR)/%.elf
	$(E) "  EEP    $@"
	$(Q)$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(HEXFORMAT) $< $@

# Create extended listing file from ELF output file.
$(OBJDIR)/%.lss: $(OBJDIR)/%.elf
	$(E) "  LSS    $<"
	$(Q)$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
$(OBJDIR)/%.sym: $(OBJDIR)/%.elf
	$(E) "  SYM    $<"
	$(Q)$(NM) -n $< > $@

# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
$(OBJDIR)/%.elf: $(OBJ) | $(OBJDIR)
	$(E) "  LINK   $@"
	$(Q)$(CC) $(CFLAGS) $^ --output $@ $(ALL_LDFLAGS)

# Compile: create object files from C source files.
$(OBJDIR)/%.o : %.c $(CONFFILES) .dep | $(OBJDIR)/src $(OBJDIR)/autoconf.h
	$(E) "  CC     $<"
	$(Q)$(CC) -c $(CFLAGS) $< -o $@

# Compile: create object files from C++ source files.
$(OBJDIR)/%.o : %.cpp $(CONFFILES) .dep | $(OBJDIR)/src $(OBJDIR)/autoconf.h
	$(E) "  CPP    $<"
	$(Q)$(CPP) -c $(CFLAGS) $< -o $@

# Compile: create assembler files from C source files.
$(OBJDIR)/%.s : %.c $(CONFFILES) .dep | $(OBJDIR)/src $(OBJDIR)/autoconf.h
	$(E) "  CC     $<"
	$(Q)$(CC) -S $(CFLAGS) $< -o $@

# Compile: create assembler files from C++ source files.
$(OBJDIR)/%.s : %.cpp $(CONFFILES) .dep | $(OBJDIR)/src $(OBJDIR)/autoconf.h
	$(CPP) -S $(CFLAGS) $< -o $@

# Assemble: create object files from assembler source files.
$(OBJDIR)/%.o : %.S $(OBJDIR)/asmconfig.h $(CONFFILES) .dep | $(OBJDIR)/src $(OBJDIR)/autoconf.h
	$(E) "  AS     $<"
	$(Q)$(CC) -c $(ASFLAGS) $< -o $@

$(OBJDIR):
	$(E) "  MKDIR  $(OBJDIR)"
	-$(Q)mkdir -p $(OBJDIR)

# Create the output directories
$(OBJDIR)/src:
	$(E) "  MKDIR  $(OBJDIRS)"
	-$(Q)mkdir -p $(OBJDIRS)

.dep:
	$(E) "  MKDIR  .dep"
	-$(Q)mkdir -p .dep

# Target: clean project.
clean:
	$(E) "  CLEAN"
	$(Q)$(REMOVE) $(TARGET).hex
	$(Q)$(REMOVE) $(TARGET).bin
	$(Q)$(REMOVE) $(TARGET).eep
	$(Q)$(REMOVE) $(TARGET).cof
	$(Q)$(REMOVE) $(TARGET).elf
	$(Q)$(REMOVE) $(TARGET).map
	$(Q)$(REMOVE) $(TARGET).sym
	$(Q)$(REMOVE) $(TARGET).lss
	$(Q)$(REMOVE) $(OBJ)
	$(Q)$(REMOVE) $(OBJDIR)/autoconf.h
	$(Q)$(REMOVE) $(OBJDIR)/asmconfig.h
	$(Q)$(REMOVE) $(OBJDIR)/*.bin
	$(Q)$(REMOVE) $(LST)
	$(Q)$(REMOVE) $(CSRC:.c=.s)
	$(Q)$(REMOVE) $(CSRC:.c=.d)
	$(Q)$(REMOVE) .dep/*
	-$(Q)rmdir --ignore-fail-on-non-empty -p $(OBJDIRS)

# Include the dependency files.
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)
-include $(wildcard .dep/*)

# Display size of file.
size:
	$(E) "  SIZE   $(TARGET).elf"
	$(Q)if [ -f $(TARGET).elf ]; then $(ELFSIZE)|grep -v debug; fi

# Listing of phony targets.
.PHONY : all build size elf hex eep lss sym clean program

