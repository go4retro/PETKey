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
 *  main.c: Main application
 */

#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "config.h"

#include "debug.h"
#include "uart.h"
#include "vkb.h"

ISR(SCAN_TIMER) {
  vkb_irq();
}

void main( void ) {
  debug_init();
  timer_init();
  vkb_init();
  sei();
  vkb_scan();
}
