/*
 *  PETKey - VIC/64 to PET keyboard adapter
 *  Copyright (C) 2021 Jim Brain and RETRO Innovations <go4retro@go4retro.com>
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
#include <util/delay.h>

#ifndef ARDUINO
 #include "autoconf.h"
#else

// Debug to serial
#define CONFIG_UART_DEBUG
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

#elif CONFIG_HARDWARE_VARIANT == 2 || defined ARDUINO_AVR_MEGA2560
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

#define XPT_RESET_DDR       DDRC
#define XPT_RESET_OUT       PORTC
#define XPT_RESET_PIN       PIN6

#define XPT_AX0_DDR         DDRC
#define XPT_AX0_OUT         PORTC
#define XPT_AX0_PIN         PIN4

#define XPT_AX1_DDR         DDRG
#define XPT_AX1_OUT         PORTG
#define XPT_AX1_PIN         PIN2

#define XPT_AX2_DDR         DDRC
#define XPT_AX2_OUT         PORTC
#define XPT_AX2_PIN         PIN0

#define XPT_AX3_DDR         DDRC
#define XPT_AX3_OUT         PORTC
#define XPT_AX3_PIN         PIN5

#define XPT_AY0_DDR         DDRC
#define XPT_AY0_OUT         PORTC
#define XPT_AY0_PIN         PIN1

#define XPT_AY1_DDR         DDRC
#define XPT_AY1_OUT         PORTC
#define XPT_AY1_PIN         PIN2

#define XPT_AY2_DDR         DDRC
#define XPT_AY2_OUT         PORTC
#define XPT_AY2_PIN         PIN7

#define XPT_DATA_DDR        DDRC
#define XPT_DATA_OUT        PORTC
#define XPT_DATA_PIN        PIN3

#define XPT_STROBE_DDR      DDRD
#define XPT_STROBE_OUT      PORTD
#define XPT_STROBE_PIN      PIN7

#define XPT_ROW_0           13
#define XPT_ROW_1           12
#define XPT_ROW_2           11
#define XPT_ROW_3           10
#define XPT_ROW_4           9
#define XPT_ROW_5           8
#define XPT_ROW_6           4
#define XPT_ROW_7           3
#define XPT_ROW_8           2
#define XPT_ROW_9           1

#define XPT_COL_0           6
#define XPT_COL_1           4
#define XPT_COL_2           7
#define XPT_COL_3           5
#define XPT_COL_4           0
#define XPT_COL_5           1
#define XPT_COL_6           2
#define XPT_COL_7           3


#else
#  error "CONFIG_HARDWARE_VARIANT is unset or set to an unknown value."
#endif

static inline void xpt_init(void) {
  XPT_STROBE_OUT &= ~_BV(XPT_STROBE_PIN);

  XPT_RESET_DDR |= _BV(XPT_RESET_PIN);
  XPT_AX0_DDR |= _BV(XPT_AX0_PIN);
  XPT_AX1_DDR |= _BV(XPT_AX1_PIN);
  XPT_AX2_DDR |= _BV(XPT_AX2_PIN);
  XPT_AX3_DDR |= _BV(XPT_AX3_PIN);
  XPT_AY0_DDR |= _BV(XPT_AY0_PIN);
  XPT_AY1_DDR |= _BV(XPT_AY1_PIN);
  XPT_AY2_DDR |= _BV(XPT_AY2_PIN);
  XPT_DATA_DDR |= _BV(XPT_DATA_PIN);
  XPT_STROBE_DDR |= _BV(XPT_STROBE_PIN);

  XPT_RESET_OUT |= _BV(XPT_RESET_PIN);
  _delay_us(1);
  XPT_RESET_OUT &= ~_BV(XPT_RESET_PIN);

}

// bits are 0 AY2,1,0,AX3,2,1,0
static inline void xpt_send(uint8_t sw, uint8_t data) {
  if(sw & 1)
    XPT_AX0_OUT |= _BV(XPT_AX0_PIN);
  else
    XPT_AX0_OUT &= ~_BV(XPT_AX0_PIN);
  if(sw & 2)
    XPT_AX1_OUT |= _BV(XPT_AX1_PIN);
  else
    XPT_AX1_OUT &= ~_BV(XPT_AX1_PIN);
  if(sw & 4)
    XPT_AX2_OUT |= _BV(XPT_AX2_PIN);
  else
    XPT_AX2_OUT &= ~_BV(XPT_AX2_PIN);
  if(sw & 8)
    XPT_AX3_OUT |= _BV(XPT_AX3_PIN);
  else
    XPT_AX3_OUT &= ~_BV(XPT_AX3_PIN);

  if(sw & 16)
    XPT_AY0_OUT |= _BV(XPT_AY0_PIN);
  else
    XPT_AY0_OUT &= ~_BV(XPT_AY0_PIN);
  if(sw & 32)
    XPT_AY1_OUT |= _BV(XPT_AY1_PIN);
  else
    XPT_AY1_OUT &= ~_BV(XPT_AY1_PIN);
  if(sw & 64)
    XPT_AY2_OUT |= _BV(XPT_AY2_PIN);
  else
    XPT_AY2_OUT &= ~_BV(XPT_AY2_PIN);

  if(data)
    XPT_DATA_OUT |= _BV(XPT_DATA_PIN);
  else
    XPT_DATA_OUT &= ~_BV(XPT_DATA_PIN);

  XPT_STROBE_OUT |= _BV(XPT_STROBE_PIN);
  _delay_us(1);
  XPT_STROBE_OUT &= ~_BV(XPT_STROBE_PIN);
  _delay_us(1);
}

#define SCAN_ROW_0          7
#define SCAN_ROW_1          1
#define SCAN_ROW_2          2
#define SCAN_ROW_3          3
#define SCAN_ROW_4          4
#define SCAN_ROW_5          5
#define SCAN_ROW_6          6
#define SCAN_ROW_7          0

#define SCAN_COL_0          7
#define SCAN_COL_1          0
#define SCAN_COL_2          5
#define SCAN_COL_3          2
#define SCAN_COL_4          3
#define SCAN_COL_5          4
#define SCAN_COL_6          1
#define SCAN_COL_7          6

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
  #define VER_TEXT           TOSTRING(VER_MAJOR) "." TOSTRING(VER_MINOR) "." \
                             TOSTRING(VER_PATCH) "." TOSTRING(VER_FIX)
#else
  #define VER_TEXT           TOSTRING(VER_MAJOR) "." TOSTRING(VER_MINOR) "." \
                             TOSTRING(VER_PATCH)
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
#  ifdef CONFIG_UART_DEBUG_RATE
#  define UART0_BAUDRATE CONFIG_UART_DEBUG_RATE
#  else
#  define UART0_BAUDRATE 57600
#  endif
#endif

#ifdef CONFIG_UART_DEBUG_SW
#  ifndef CONFIG_UART_DEBUG_SW_PORT
#    define CONFIG_UART_DEBUG_SW_PORT 1
#  endif
#endif

#define UART_DOUBLE_SPEED

#endif /*CONFIG_H*/
