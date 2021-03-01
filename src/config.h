/*
 *  PETKey - VIC/64 to PET keyboard adapter
 *  Copyright (C) 2021  Jim Brain and RETRO Innovations <go4retro@go4retro.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; version 2 of the License only.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  config.h: User-configurable options to simplify hardware changes and/or
 *             reduce the code/ram requirements of the code.
 */

#ifndef CONFIG_H
#define CONFIG_H

#define FLASH_MEM_DATA  1

#include <avr/io.h>

#if defined ARDUINO_AVR_UNO || defined ARDUINO_AVR_PRO || defined ARDUINO_AVR_NANO
 #define CONFIG_HARDWARE_VARIANT   3 // Hardware variant 3 is Arduino, with BAV on D2 for wakeup from standby mode.
 // Variant 3 has been tested on Pro Mini, Uno, and Nano as functional.  Select target platform in the IDE.
#endif

#ifndef ARDUINO
 #include "autoconf.h"
#else

// Debug to serial
//#define CONFIG_UART_DEBUG
//#define CONFIG_UART_DEBUG_SW
#define CONFIG_UART_DEBUG_RATE    115200
#define CONFIG_UART_DEBUG_FLUSH
#define CONFIG_UART_BUF_SHIFT     8

#endif

#ifndef TRUE
#define FALSE                 0
#define TRUE                  (!FALSE)
#endif

//#define ENABLE_UART0
// log2 of the UART buffer size, i.e. 6 for 64, 7 for 128, 8 for 256 etc.
//#define UART0_TX_BUFFER_SHIFT  6
//#define UART0_RX_BUFFER_SHIFT  6
//#define UART0_BAUDRATE CONFIG_UART_BAUDRATE

#if CONFIG_HARDWARE_VARIANT == 1
/* ---------- Hardware configuration: PETKey Final ---------- */

#define KB_ROW_HI_OUT       PORTB
#define KB_ROW_HI_IN        PINB
#define KB_ROW_HI_DDR       DDRB
#define KB_ROW_LO_OUT       PORTC
#define KB_ROW_LO_IN        PINC
#define KB_ROW_LO_DDR       DDRC
#define KB_COL_OUT          PORTA
#define KB_COL_IN           PINA
#define KB_COL_DDR          DDRA

#define XPT_PORT_DATA_OUT   PORTB
#define XPT_DDR_DATA        DDRB
#define XPT_PORT_STROBE_OUT PORTD
#define XPT_DDR_STROBE      DDRD

#define XPT_PIN_STROBE      (1<<PIN6)

#elif CONFIG_HARDWARE_VARIANT == 2 || defined ARDUINO_AVR_MEGA
/* ---------- Hardware configuration: PETKey Arduino ---------- */

#define SCAN_TIMER          TIMER0_COMPA_vect

static inline void timer_init(void) {
  // need to scan 120 * 8 times a sec
  TCCR0A = _BV(WGM01);            // CTC mode
  TCCR0B = _BV(CS02) | _BV(CS00); // /1024
  OCR0A = (F_CPU / 1024 / 120 / 8) - 1;
  TIMSK0 = _BV(OCIE0A);
}

// rmeove hi port.
#define KB_ROW_LO_OUT       PORTL
#define KB_ROW_LO_IN        PINL
#define KB_ROW_LO_DDR       DDRL
#define KB_COL_OUT          PORTA
#define KB_COL_IN           PINA
#define KB_COL_DDR          DDRA

#define XPT_PORT_DATA_OUT   PORTB
#define XPT_DDR_DATA        DDRB
#define XPT_PORT_STROBE_OUT PORTD
#define XPT_DDR_STROBE      DDRD

#define XPT_PIN_STROBE      _BV(PIN6)

#else
#  error "CONFIG_HARDWARE_VARIANT is unset or set to an unknown value."
#endif

#define SCAN_MAP(r,c)             ((r == 0 ? 7 : \
                                  r == 1 ? 1 : \
                                  r == 2 ? 2 : \
                                  r == 3 ? 3 : \
                                  r == 4 ? 4 : \
                                  r == 5 ? 5 : \
                                  r == 6 ? 6 : 0) | \
                                  (c == 0 ? 7 : \
                                  c == 1 ? 0 : \
                                  c == 2 ? 5 : \
                                  c == 3 ? 2 : \
                                  c == 4 ? 3 : \
                                  c == 5 ? 4 : \
                                  c == 6 ? 1 : 6) << 3)

#define KB_MAX_ROWS         8

static inline void kb_set_row(uint8_t row) {
  uint8_t tmp;

  tmp = _BV(row);
  KB_ROW_LO_DDR = tmp;          // bring DDR high on the one row
  KB_ROW_LO_OUT = (uint8_t)~tmp;// bring others to pullups.
}

static inline uint8_t kb_read_col(void) {
  return (uint8_t) ~KB_COL_IN;
}

#include "version.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef VER_PATCH
#ifdef VER_FIX
  #define VER_TEXT           TOSTRING(VER_MAJOR) "." TOSTRING(VER_MINOR) "." TOSTRING(VER_PATCH) "." TOSTRING(VER_FIX)
#else
  #define VER_TEXT           TOSTRING(VER_MAJOR) "." TOSTRING(VER_MINOR) "." TOSTRING(VER_PATCH)
#endif
#else
  #define VER_TEXT           TOSTRING(VER_MAJOR) "." TOSTRING(VER_MINOR)
#endif
#ifdef VER_PRERELEASE
  #define VERSION "" VER_TEXT TOSTRING(VER_PRERELEASE) ""
#else
  #define VERSION "" VER_TEXT ""
#endif

#ifdef CONFIG_UART_DEBUG
#  define UART0_ENABLE
#  ifdef CONFIG_UART_BAUDRATE
#  define UART0_BAUDRATE CONFIG_UART_BAUDRATE
#  else
#  define UART0_BAUDRATE 57600
#  endif
#endif

#ifdef CONFIG_UART_DEBUG_SW
#  ifndef CONFIG_UART_DEBUG_SW_PORT
#    define CONFIG_UART_DEBUG_SW_PORT 1
#  endif
#endif


#endif /*CONFIG_H*/
