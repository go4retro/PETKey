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
  XPT_PORT_DATA_OUT=((state!=FALSE) | sw);
  // strobe STROBE PIN
  XPT_PORT_STROBE_OUT|=XPT_PIN_STROBE;
  // bring low
  XPT_PORT_STROBE_OUT&=(uint8_t)~XPT_PIN_STROBE;
}

void reset_matrix(void) {
  uint8_t i=0;

  // reset switches...
  do {
    set_switch(i+=2,FALSE);
  } while (i!=0);
}

void vkb_init(void) {
  kb_init();
  XPT_PORT_STROBE_OUT&=(uint8_t)~XPT_PIN_STROBE;
  XPT_DDR_STROBE|=XPT_PIN_STROBE;
  XPT_DDR_DATA=0xff;

  reset_matrix();

}

#define KB_SCAN_CODE_MASK 0x7f
#define META_LSHIFT       0x01
#define META_RSHIFT       0x02
#define META_CONTROL      0x02
#define META_SHIFT_MASK   (META_LSHIFT | META_RSHIFT)

#define MOD_OVERRIDE      0x80

void set_vkey(uint8_t unshifted, uint8_t shifted, uint8_t control, uint8_t state) {

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
    } else if(meta & META_CONTROL) {    // control is on
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
    } else if(meta & META_CONTROL) {    // control is on
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
  case SCAN_C64_KEY_DELETE:
    debug_puts("DELETE");
    set_vkey(MAT_PET_KEY_DELETE, MAT_PET_KEY_DELETE, MAT_PET_KEY_DELETE, state);
    break;
  case SCAN_C64_KEY_RETURN:
    debug_puts("RETURN");
    set_vkey(MAT_PET_KEY_RETURN, MAT_PET_KEY_RETURN, MAT_PET_KEY_RETURN, state);
    break;
  case SCAN_C64_KEY_CRSR_RIGHT:
    debug_puts("CSRT");
    set_vkey(MAT_PET_KEY_CRSR_RIGHT, MAT_PET_KEY_CRSR_RIGHT, MAT_PET_KEY_CRSR_RIGHT, state);
    break;
  case SCAN_C64_KEY_F7:
    break;
  case SCAN_C64_KEY_F1:
    break;
  case SCAN_C64_KEY_F3:
    break;
  case SCAN_C64_KEY_F5:
    break;
  case SCAN_C64_KEY_CRSR_DOWN:
    debug_puts("CSDN");
    set_vkey(MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_CRSR_DOWN, state);
    break;

  case SCAN_C64_KEY_3:
    debug_putc('3');
    set_vkey(MAT_PET_KEY_3, MAT_PET_KEY_3, MAT_PET_KEY_3, state);
    break;
  case SCAN_C64_KEY_W:
    debug_putc('W');
    set_vkey(MAT_PET_KEY_W, MAT_PET_KEY_W, MAT_PET_KEY_W, state);
    break;
  case SCAN_C64_KEY_A:
    debug_putc('A');
    set_vkey(MAT_PET_KEY_A, MAT_PET_KEY_A, MAT_PET_KEY_A, state);
    break;
  case SCAN_C64_KEY_4:
    debug_putc('4');
    set_vkey(MAT_PET_KEY_4, MAT_PET_KEY_4, MAT_PET_KEY_4, state);
    break;
  case SCAN_C64_KEY_Z:
    debug_putc('Z');
    set_vkey(MAT_PET_KEY_Z, MAT_PET_KEY_Z, MAT_PET_KEY_Z, state);
    break;
  case SCAN_C64_KEY_S:
    debug_putc('S');
    set_vkey(MAT_PET_KEY_S, MAT_PET_KEY_S, MAT_PET_KEY_S, state);
    break;
  case SCAN_C64_KEY_E:
    debug_putc('E');
    set_vkey(MAT_PET_KEY_E, MAT_PET_KEY_E, MAT_PET_KEY_E, state);
    break;
  case SCAN_C64_KEY_LSHIFT:
    debug_puts("LSHIFT");
    set_vkey(MAT_PET_KEY_LSHIFT, MAT_PET_KEY_LSHIFT, MAT_PET_KEY_LSHIFT, state);
    if(state)
      meta |= META_LSHIFT;
    else
      meta &= ~META_LSHIFT;
    break;

  case SCAN_C64_KEY_5:
    debug_putc('5');
    set_vkey(MAT_PET_KEY_5, MAT_PET_KEY_5, MAT_PET_KEY_5, state);
    break;
  case SCAN_C64_KEY_R:
    debug_putc('R');
    set_vkey(MAT_PET_KEY_R, MAT_PET_KEY_R, MAT_PET_KEY_R, state);
    break;
  case SCAN_C64_KEY_D:
    debug_putc('D');
    set_vkey(MAT_PET_KEY_D, MAT_PET_KEY_R, MAT_PET_KEY_R, state);
    break;
  case SCAN_C64_KEY_6:
    debug_putc('6');
    set_vkey(MAT_PET_KEY_6, MAT_PET_KEY_6, MAT_PET_KEY_6, state);
    break;
  case SCAN_C64_KEY_C:
    debug_putc('C');
    set_vkey(MAT_PET_KEY_C, MAT_PET_KEY_C, MAT_PET_KEY_C, state);
    break;
  case SCAN_C64_KEY_F:
    debug_putc('F');
    set_vkey(MAT_PET_KEY_F, MAT_PET_KEY_F, MAT_PET_KEY_F, state);
    break;
  case SCAN_C64_KEY_X:
    debug_putc('X');
    set_vkey(MAT_PET_KEY_X, MAT_PET_KEY_X, MAT_PET_KEY_X, state);
    break;
  case SCAN_C64_KEY_T:
    debug_putc('T');
    set_vkey(MAT_PET_KEY_T, MAT_PET_KEY_T, MAT_PET_KEY_T, state);
    break;

  case SCAN_C64_KEY_7:
    debug_putc('7');
    set_vkey(MAT_PET_KEY_7, MAT_PET_KEY_7, MAT_PET_KEY_7, state);
    break;
  case SCAN_C64_KEY_Y:
    debug_putc('Y');
    set_vkey(MAT_PET_KEY_Y, MAT_PET_KEY_Y, MAT_PET_KEY_Y, state);
    break;
  case SCAN_C64_KEY_G:
    debug_putc('G');
    set_vkey(MAT_PET_KEY_G, MAT_PET_KEY_G, MAT_PET_KEY_G, state);
    break;
  case SCAN_C64_KEY_8:
    debug_putc('8');
    set_vkey(MAT_PET_KEY_8, MAT_PET_KEY_8, MAT_PET_KEY_8, state);
    break;
  case SCAN_C64_KEY_B:
    debug_putc('B');
    set_vkey(MAT_PET_KEY_B, MAT_PET_KEY_B, MAT_PET_KEY_B, state);
    break;
  case SCAN_C64_KEY_H:
    debug_putc('H');
    set_vkey(MAT_PET_KEY_H, MAT_PET_KEY_H, MAT_PET_KEY_H, state);
    break;
  case SCAN_C64_KEY_U:
    debug_putc('U');
    set_vkey(MAT_PET_KEY_U, MAT_PET_KEY_U, MAT_PET_KEY_U, state);
    break;
  case SCAN_C64_KEY_V:
    debug_putc('V');
    set_vkey(MAT_PET_KEY_V, MAT_PET_KEY_V, MAT_PET_KEY_V, state);
    break;

  case SCAN_C64_KEY_9:
    debug_putc('9');
    set_vkey(MAT_PET_KEY_9, MAT_PET_KEY_9, MAT_PET_KEY_9, state);
    break;
  case SCAN_C64_KEY_I:
    debug_putc('I');
    set_vkey(MAT_PET_KEY_I, MAT_PET_KEY_I, MAT_PET_KEY_I, state);
    break;
  case SCAN_C64_KEY_J:
    debug_putc('J');
    set_vkey(MAT_PET_KEY_J, MAT_PET_KEY_J, MAT_PET_KEY_J, state);
    break;
  case SCAN_C64_KEY_0:
    debug_putc('0');
    set_vkey(MAT_PET_KEY_0, MAT_PET_KEY_0, MAT_PET_KEY_0, state);
    break;
  case SCAN_C64_KEY_M:
    debug_putc('M');
    set_vkey(MAT_PET_KEY_M, MAT_PET_KEY_M, MAT_PET_KEY_M, state);
    break;
  case SCAN_C64_KEY_K:
    debug_putc('K');
    set_vkey(MAT_PET_KEY_K, MAT_PET_KEY_K, MAT_PET_KEY_K, state);
    break;
  case SCAN_C64_KEY_O:
    debug_putc('O');
    set_vkey(MAT_PET_KEY_O, MAT_PET_KEY_O, MAT_PET_KEY_O, state);
    break;
  case SCAN_C64_KEY_N:
    debug_putc('N');
    set_vkey(MAT_PET_KEY_N, MAT_PET_KEY_N, MAT_PET_KEY_N, state);
    break;

  case SCAN_C64_KEY_PLUS:
    debug_putc('+');
    set_vkey(MAT_PET_KEY_PLUS, MAT_PET_KEY_PLUS, MAT_PET_KEY_PLUS, state);
    break;
  case SCAN_C64_KEY_P:
    debug_putc('P');
    set_vkey(MAT_PET_KEY_P, MAT_PET_KEY_P, MAT_PET_KEY_P, state);
    break;
  case SCAN_C64_KEY_L:
    debug_putc('L');
    set_vkey(MAT_PET_KEY_L, MAT_PET_KEY_L, MAT_PET_KEY_L, state);
    break;
  case SCAN_C64_KEY_MINUS:
    debug_putc('-');
    set_vkey(MAT_PET_KEY_MINUS, MAT_PET_KEY_MINUS, MAT_PET_KEY_MINUS, state);
    break;
  case SCAN_C64_KEY_PERIOD:
    debug_putc('.');
    set_vkey(MAT_PET_KEY_PERIOD, MAT_PET_KEY_PERIOD, MAT_PET_KEY_PERIOD, state);
    break;
  case SCAN_C64_KEY_COLON:
    debug_putc(':');
    set_vkey(MAT_PET_KEY_COLON, MAT_PET_KEY_COLON, MAT_PET_KEY_COLON, state);
    break;
  case SCAN_C64_KEY_AT:
    break;
  case SCAN_C64_KEY_COMMA:
    debug_putc(',');
    set_vkey(MAT_PET_KEY_COMMA, MAT_PET_KEY_COMMA, MAT_PET_KEY_COMMA, state);
    break;

  case SCAN_C64_KEY_POUND:
    break;
  case SCAN_C64_KEY_ASTERIX:
    debug_putc('*');
    set_vkey(MAT_PET_KEY_ASTERIX, MAT_PET_KEY_ASTERIX, MAT_PET_KEY_ASTERIX, state);
    break;
  case SCAN_C64_KEY_SEMICOLON:
    break;
  case SCAN_C64_KEY_HOME:
    debug_puts("HOME");
    set_vkey(MAT_PET_KEY_HOME, MAT_PET_KEY_HOME, MAT_PET_KEY_HOME, state);
    break;
  case SCAN_C64_KEY_RSHIFT:
    debug_puts("RSHIFT");
    set_vkey(MAT_PET_KEY_RSHIFT, MAT_PET_KEY_RSHIFT, MAT_PET_KEY_RSHIFT, state);
    if(state)
      meta |= META_RSHIFT;
    else
      meta &= ~META_RSHIFT;
    break;
  case SCAN_C64_KEY_EQUALS:
    debug_putc('=');
    set_vkey(MAT_PET_KEY_EQUALS, MAT_PET_KEY_EQUALS, MAT_PET_KEY_EQUALS, state);
    break;
  case SCAN_C64_KEY_UP_ARROW:
    debug_putc('^');
    set_vkey(MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_UP_ARROW, state);
    break;
  case SCAN_C64_KEY_SLASH:
    debug_putc('/');
    set_vkey(MAT_PET_KEY_SLASH, MAT_PET_KEY_SLASH, MAT_PET_KEY_SLASH, state);
    break;

  case SCAN_C64_KEY_1:
    debug_putc('1');
    set_vkey(MAT_PET_KEY_1, MAT_PET_KEY_1, MAT_PET_KEY_1, state);
    break;
  case SCAN_C64_KEY_LEFT_ARROW:
    debug_puts("BACKARROW");
    set_vkey(MAT_PET_KEY_LEFT_ARROW, MAT_PET_KEY_LEFT_ARROW, MAT_PET_KEY_LEFT_ARROW, state);
    break;
  case SCAN_C64_KEY_CTRL:
    debug_puts("CTRL");
    // no key to depress
    if(state)
      meta |= META_CONTROL;
    else
      meta &= ~META_CONTROL;
    break;
  case SCAN_C64_KEY_2:
    debug_putc('2');
    set_vkey(MAT_PET_KEY_2, MAT_PET_KEY_DOUBLE_QUOTE | MOD_OVERRIDE, MAT_PET_KEY_2, state);
    break;
  case SCAN_C64_KEY_SPACE:
    debug_putc(' ');
    set_vkey(MAT_PET_KEY_SPACE, MAT_PET_KEY_SPACE, MAT_PET_KEY_SPACE, state);
    break;
  case SCAN_C64_KEY_CBM:
    break;
  case SCAN_C64_KEY_Q:
    debug_putc('Q');
    set_vkey(MAT_PET_KEY_Q, MAT_PET_KEY_Q, MAT_PET_KEY_Q, state);
    break;
  case SCAN_C64_KEY_RUN_STOP:
    debug_puts("RUN/STOP");
    set_vkey(MAT_PET_KEY_RUN_STOP, MAT_PET_KEY_RUN_STOP, MAT_PET_KEY_RUN_STOP, state);
    break;
  }
}

void vkb_scan(void) {
  uint8_t key;

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
