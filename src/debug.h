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
 *  debug.h: Definitions for the UAR debug routines
 */

#ifndef DEBUG_H
#define DEBUG_H

#define debug_puts_P(x) _debug_puts_P(PSTR(x))

#if defined CONFIG_UART_DEBUG || defined CONFIG_UART_DEBUG_SW || defined ARDUINO_UART_DEBUG
void debug_putc(uint8_t data);
void debug_puts(const char *text);
void _debug_puts_P(const char *text);
void debug_puthex(uint8_t hex);
void debug_putcrlf(void);
void debug_trace(void *ptr, uint16_t start, uint16_t len);
void debug_init(void);
#else
#define debug_init()            do {} while(0)
#define debug_putc(x)           do {} while(0)
#define debug_puthex(x)         do {} while(0)
#define debug_trace(x,y,z)      do {} while(0)
#define debug_puts(x)           do {} while(0)
#define _debug_puts_P(x)        do {} while(0)
#define debug_putcrlf()         do {} while(0)
#endif
#endif
