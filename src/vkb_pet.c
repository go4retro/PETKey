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
 *  vkb_pet.c: VIC/64 to PET conversion
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <inttypes.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <util/delay.h>

#include "config.h"

#include "debug.h"
#include "eeprom.h"
#include "kb.h"
#include "uart.h"
#include "vkb.h"

#include "vkb_pet.h"

static uint8_t meta;

void vkb_irq(void) {
  kb_scan();
  DDRB |= _BV(PIN7);
  PORTB ^= _BV(PIN7);
}

void set_switch(uint8_t sw, uint8_t state) {
  xpt_send(sw,state);
}

void vkb_init(void) {
  kb_init();
  xpt_init();
}


#define KB_SCAN_CODE_MASK 0x7f
#define META_LSHIFT       0x01
#define META_RSHIFT       0x02
#define META_CMDR      0x02
#define META_SHIFT_MASK   (META_LSHIFT | META_RSHIFT)

#define MOD_OVERRIDE      0x80

void set_vkey(uint8_t unshifted, uint8_t shifted, uint8_t control, uint8_t state) {
  // TODO optimize for MAT_PET_KEY_NONE

  if(state) {                           // key press
    if(!(meta & META_SHIFT_MASK)) {     // shift is not on
      if(unshifted & MOD_OVERRIDE) {    // shift off and virtual shift needed
        set_switch(MAT_PET_KEY_LSHIFT, state);
      }
      set_switch(unshifted & ~MOD_OVERRIDE, state);
    } else if(meta & META_SHIFT_MASK) { // shift is on
      if(shifted & MOD_OVERRIDE) {      // shift on and virtual unshift needed)
        if(meta && META_LSHIFT)         // do I need to unshift left?
          set_switch(MAT_PET_KEY_LSHIFT, FALSE);
        if(meta && META_RSHIFT)         // do I need to unshift right?
          set_switch(MAT_PET_KEY_RSHIFT, FALSE);
      }
      set_switch(shifted & ~MOD_OVERRIDE, state);
    } else if(meta & META_CMDR) {    // control is on
      if(!(meta & META_SHIFT_MASK)) {   // shift is not on
        if(control & MOD_OVERRIDE) {    // shift off and virtual shift needed
          set_switch(MAT_PET_KEY_LSHIFT, state);
        }
      } else if(meta & META_SHIFT_MASK) { // shift is on
        if(!(control & MOD_OVERRIDE)) {   // shift on and virtual unshift needed
          if(meta && META_LSHIFT)         // do I need to unshift left?
            set_switch(MAT_PET_KEY_LSHIFT, FALSE);
          if(meta && META_RSHIFT)         // do I need to unshift right?
            set_switch(MAT_PET_KEY_RSHIFT, FALSE);
        }
      }
      set_switch(control & ~MOD_OVERRIDE, state);
    }
  } else {                              // key release
    if(!(meta & META_SHIFT_MASK)) {     // shift is not on.
      if(unshifted & MOD_OVERRIDE) {    // shift off and virtual shift is on
        set_switch(MAT_PET_KEY_LSHIFT, state);
      }
      set_switch(unshifted & ~MOD_OVERRIDE, state);
    } else if(meta & META_SHIFT_MASK) { // shift is on
      if(shifted & MOD_OVERRIDE) {      // shift on and virtual unshift needed)
        if(meta && META_LSHIFT)         // do I need to shift left?
          set_switch(MAT_PET_KEY_LSHIFT, TRUE);
        if(meta && META_RSHIFT)         // do I need to shift right?
          set_switch(MAT_PET_KEY_RSHIFT, TRUE);
      }
      set_switch(shifted & ~MOD_OVERRIDE, state);
    } else if(meta & META_CMDR) {    // control is on
      if(!(meta & META_SHIFT_MASK)) {   // shift is not on
        if(control & MOD_OVERRIDE) {    // shift off and virtual shift not needed
          set_switch(MAT_PET_KEY_LSHIFT, state);
        }
      } else if(meta & META_SHIFT_MASK) { // shift is on
        if(!(control & MOD_OVERRIDE)) {   // shift on and virtual unshift not needed
          if(meta && META_LSHIFT)         // do I need to shift left?
            set_switch(MAT_PET_KEY_LSHIFT, TRUE);
          if(meta && META_RSHIFT)         // do I need to shift right?
            set_switch(MAT_PET_KEY_RSHIFT, TRUE);
        }
      }
      set_switch(control & ~MOD_OVERRIDE, state);
    }
  }
}


static void map_key(uint8_t key) {
  uint8_t state;

  state = (key & KB_KEY_UP ? FALSE : TRUE);

  //debug_puthex(key & KB_SCAN_CODE_MASK);
  switch(key & KB_SCAN_CODE_MASK) {
  case SCAN_C64_KEY_1:
    debug_putc('1');
    set_vkey(MAT_PET_KEY_1, MAT_PET_KEY_1, MAT_PET_KEY_1, state);
    break;
  case SCAN_C64_KEY_2:
    debug_putc('2');
    set_vkey(MAT_PET_KEY_2, MAT_PET_KEY_DOUBLE_QUOTE | MOD_OVERRIDE, MAT_PET_KEY_2, state);
    break;
  case SCAN_C64_KEY_3:
    debug_putc('3');
    set_vkey(MAT_PET_KEY_3, MAT_PET_KEY_HASH | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_4:
    debug_putc('4');
    set_vkey(MAT_PET_KEY_4, MAT_PET_KEY_DOLLAR_SIGN | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_5:
    debug_putc('5');
    set_vkey(MAT_PET_KEY_5, MAT_PET_KEY_PERCENT | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_6:
    debug_putc('6');
    set_vkey(MAT_PET_KEY_6, MAT_PET_KEY_AMPERSAND | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_7:
    debug_putc('7');
    set_vkey(MAT_PET_KEY_7, MAT_PET_KEY_APOSTROPHE | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_8:
    debug_putc('8');
    set_vkey(MAT_PET_KEY_8, MAT_PET_KEY_LEFT_PAREN | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_9:
    debug_putc('9');
    set_vkey(MAT_PET_KEY_9, MAT_PET_KEY_LEFT_PAREN | MOD_OVERRIDE, MAT_PET_KEY_REVERSE, state);
    break;
  case SCAN_C64_KEY_0:
    debug_putc('0');
    set_vkey(MAT_PET_KEY_0, MAT_PET_KEY_0, MAT_PET_KEY_REVERSE | MOD_OVERRIDE, state);
    break;

  case SCAN_C64_KEY_DELETE:
    debug_puts("DELETE");
    set_vkey(MAT_PET_KEY_DELETE, MAT_PET_KEY_DELETE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_RETURN:
    debug_puts("RETURN");
    set_vkey(MAT_PET_KEY_RETURN, MAT_PET_KEY_RETURN, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_CRSR_RIGHT:
    debug_puts("CSRT");
    set_vkey(MAT_PET_KEY_CRSR_RIGHT, MAT_PET_KEY_CRSR_RIGHT, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_F7:
    debug_puts("F7");
    set_vkey(MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_F1:
    debug_puts("F1");
    set_vkey(MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_F3:
    debug_puts("F3");
    set_vkey(MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_F5:
    debug_puts("F5");
    set_vkey(MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_CRSR_DOWN:
    debug_puts("CSDN");
    set_vkey(MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_NONE, state);
    break;

  case SCAN_C64_KEY_W:
    debug_putc('W');
    set_vkey(MAT_PET_KEY_W, MAT_PET_KEY_W, MAT_PET_KEY_3 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_A:
    debug_putc('A');
    set_vkey(MAT_PET_KEY_A, MAT_PET_KEY_A, MAT_PET_KEY_0 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_Z:
    debug_putc('Z');
    set_vkey(MAT_PET_KEY_Z, MAT_PET_KEY_Z, MAT_PET_KEY_MINUS | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_S:
    debug_putc('S');
    set_vkey(MAT_PET_KEY_S, MAT_PET_KEY_S, MAT_PET_KEY_PERIOD | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_E:
    debug_putc('E');
    set_vkey(MAT_PET_KEY_E, MAT_PET_KEY_E, MAT_PET_KEY_1 | MOD_OVERRIDE, state);
    break;

  case SCAN_C64_KEY_R:
    debug_putc('R');
    set_vkey(MAT_PET_KEY_R, MAT_PET_KEY_R, MAT_PET_KEY_2 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_D:
    debug_putc('D');
    set_vkey(MAT_PET_KEY_D, MAT_PET_KEY_D, MAT_PET_KEY_COMMA | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_C:
    debug_putc('C');
    set_vkey(MAT_PET_KEY_C, MAT_PET_KEY_C, MAT_PET_KEY_LESS_THAN | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_F:
    debug_putc('F');
    set_vkey(MAT_PET_KEY_F, MAT_PET_KEY_F, MAT_PET_KEY_SEMICOLON | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_X:
    debug_putc('X');
    set_vkey(MAT_PET_KEY_X, MAT_PET_KEY_X, MAT_PET_KEY_EQUALS | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_T:
    debug_putc('T');
    set_vkey(MAT_PET_KEY_T, MAT_PET_KEY_T, MAT_PET_KEY_HASH | MOD_OVERRIDE, state);
    break;

  case SCAN_C64_KEY_Y:
    debug_putc('Y');
    set_vkey(MAT_PET_KEY_Y, MAT_PET_KEY_Y, MAT_PET_KEY_7 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_G:
    debug_putc('G');
    set_vkey(MAT_PET_KEY_G, MAT_PET_KEY_G, MAT_PET_KEY_PERCENT | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_B:
    debug_putc('B');
    set_vkey(MAT_PET_KEY_B, MAT_PET_KEY_B, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_H:
    debug_putc('H');
    set_vkey(MAT_PET_KEY_H, MAT_PET_KEY_H, MAT_PET_KEY_4 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_U:
    debug_putc('U');
    set_vkey(MAT_PET_KEY_U, MAT_PET_KEY_U, MAT_PET_KEY_8 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_V:
    debug_putc('V');
    set_vkey(MAT_PET_KEY_V, MAT_PET_KEY_V, MAT_PET_KEY_GREATER_THAN | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_I:
    debug_putc('I');
    set_vkey(MAT_PET_KEY_I, MAT_PET_KEY_I, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_J:
    debug_putc('J');
    set_vkey(MAT_PET_KEY_J, MAT_PET_KEY_J, MAT_PET_KEY_5 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_M:
    debug_putc('M');
    set_vkey(MAT_PET_KEY_M, MAT_PET_KEY_M, MAT_PET_KEY_APOSTROPHE | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_K:
    debug_putc('K');
    set_vkey(MAT_PET_KEY_K, MAT_PET_KEY_K, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_O:
    debug_putc('O');
    set_vkey(MAT_PET_KEY_O, MAT_PET_KEY_O, MAT_PET_KEY_8 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_N:
    debug_putc('N');
    set_vkey(MAT_PET_KEY_N, MAT_PET_KEY_N, MAT_PET_KEY_ASTERIX | MOD_OVERRIDE, state);
    break;

  case SCAN_C64_KEY_PLUS:
    debug_putc('+');
    set_vkey(MAT_PET_KEY_PLUS, MAT_PET_KEY_LEFT_BRACKET, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_P:
    debug_putc('P');
    set_vkey(MAT_PET_KEY_P, MAT_PET_KEY_P, MAT_PET_KEY_9 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_L:
    debug_putc('L');
    set_vkey(MAT_PET_KEY_L, MAT_PET_KEY_L, MAT_PET_KEY_6 | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_Q:
    debug_putc('Q');
    set_vkey(MAT_PET_KEY_Q, MAT_PET_KEY_Q, MAT_PET_KEY_PLUS | MOD_OVERRIDE, state);
    break;

  case SCAN_C64_KEY_MINUS:
    debug_putc('-');
    set_vkey(MAT_PET_KEY_MINUS, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_PERIOD:
    debug_putc('.');
    set_vkey(MAT_PET_KEY_PERIOD, MAT_PET_KEY_GREATER_THAN | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_COLON:
    debug_putc(':');
    set_vkey(MAT_PET_KEY_COLON, MAT_PET_KEY_LEFT_BRACKET | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_AT:
    debug_putc('@');
    set_vkey(MAT_PET_KEY_AT, MAT_PET_KEY_COLON | MOD_OVERRIDE, MAT_PET_KEY_SLASH | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_COMMA:
    debug_putc(',');
    set_vkey(MAT_PET_KEY_COMMA, MAT_PET_KEY_LESS_THAN | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_POUND:
    debug_puts("POUND");
    set_vkey(MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, MAT_PET_KEY_RIGHT_PAREN | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_ASTERIX:
    debug_putc('*');
    set_vkey(MAT_PET_KEY_ASTERIX, MAT_PET_KEY_AMPERSAND | MOD_OVERRIDE, MAT_PET_KEY_LEFT_ARROW | MOD_OVERRIDE, state);
    break;
  case SCAN_C64_KEY_SEMICOLON:
    debug_putc(';');
    set_vkey(MAT_PET_KEY_NONE, MAT_PET_KEY_RIGHT_BRACKET | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_HOME:
    debug_puts("HOME");
    set_vkey(MAT_PET_KEY_HOME, MAT_PET_KEY_HOME, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_EQUALS:
    debug_putc('=');
    set_vkey(MAT_PET_KEY_EQUALS, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_UP_ARROW:
    debug_putc('^');
    set_vkey(MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_SLASH:
    debug_putc('/');
    set_vkey(MAT_PET_KEY_SLASH, MAT_PET_KEY_QUESTION_MARK | MOD_OVERRIDE, MAT_PET_KEY_NONE, state);
    break;

  case SCAN_C64_KEY_LEFT_ARROW:
    debug_puts("BACKARROW");
    set_vkey(MAT_PET_KEY_LEFT_ARROW, MAT_PET_KEY_NONE, MAT_PET_KEY_LEFT_ARROW, state);
    break;
  case SCAN_C64_KEY_SPACE:
    debug_putc(' ');
    set_vkey(MAT_PET_KEY_SPACE, MAT_PET_KEY_SPACE, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_RUN_STOP:
    debug_puts("RUN/STOP");
    set_vkey(MAT_PET_KEY_RUN_STOP, MAT_PET_KEY_RUN_STOP, MAT_PET_KEY_NONE, state);
    break;

  case SCAN_C64_KEY_LSHIFT:
    debug_puts("LSHIFT");
    set_vkey(MAT_PET_KEY_LSHIFT, MAT_PET_KEY_LSHIFT, MAT_PET_KEY_LSHIFT, state);
    if(state)
      meta |= META_LSHIFT;
    else
      meta &= ~META_LSHIFT;
    break;
  case SCAN_C64_KEY_RSHIFT:
    debug_puts("RSHIFT");
    set_vkey(MAT_PET_KEY_RSHIFT, MAT_PET_KEY_RSHIFT, MAT_PET_KEY_RSHIFT, state);
    if(state)
      meta |= META_RSHIFT;
    else
      meta &= ~META_RSHIFT;
    break;
  case SCAN_C64_KEY_CBM:
    debug_puts("CBM");
    // no key to depress
    if(state)
      meta |= META_CMDR;
    else
      meta &= ~META_CMDR;
    break;

  case SCAN_C64_KEY_CTRL:
    debug_puts("CTRL");
    set_vkey(MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE, state);
    break;
  }
}

void vkb_scan(void) {
  uint8_t key;

  for(uint8_t r = 0; r < 10; r++) {
    for(uint8_t c = 0; c < 8; c++) {
      debug_puthex(r);
      debug_putc(':');
      debug_putcrlf();
      debug_puthex(c);
      debug_putcrlf();
      xpt_send(r | c << 4,TRUE);
      _delay_ms(1000);
      xpt_send(r | c << 4,FALSE);
      _delay_ms(1000);
    }
  }
  while(1);

  for(;;) {
    if(kb_data_available() != 0) {
      // kb sent data...
      key=kb_recv();
      map_key(key);
      //debug_puthex(key>>3);
      //debug_putc('|');
      //debug_puthex(key & 0x07);
      //debug_putc('-');
    }
  }
}
