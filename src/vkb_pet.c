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
  //DDRB |= _BV(PIN7);
  //PORTB ^= _BV(PIN7);
}

#define KB_SCAN_CODE_MASK 0x7f
#define META_LSHIFT       0x01
#define META_RSHIFT       0x02
#define META_CMDR         0x04
#define META_SHIFT_MASK   (META_LSHIFT | META_RSHIFT)

#define MOD_OVERRIDE      0x80

static uint8_t ascii_map[] = {
                              MAT_PET_KEY_SPACE,
                              MAT_PET_KEY_EXCLAMATION,
                              MAT_PET_KEY_DOUBLE_QUOTE,
                              MAT_PET_KEY_HASH,
                              MAT_PET_KEY_DOLLAR_SIGN,
                              MAT_PET_KEY_PERCENT,
                              MAT_PET_KEY_AMPERSAND,
                              MAT_PET_KEY_APOSTROPHE,
                              MAT_PET_KEY_LEFT_PAREN,
                              MAT_PET_KEY_RIGHT_PAREN,
                              MAT_PET_KEY_ASTERIX,
                              MAT_PET_KEY_PLUS,
                              MAT_PET_KEY_COMMA,
                              MAT_PET_KEY_MINUS,
                              MAT_PET_KEY_PERIOD,
                              MAT_PET_KEY_SLASH,
                              MAT_PET_KEY_0,
                              MAT_PET_KEY_1,
                              MAT_PET_KEY_2,
                              MAT_PET_KEY_3,
                              MAT_PET_KEY_4,
                              MAT_PET_KEY_5,
                              MAT_PET_KEY_6,
                              MAT_PET_KEY_7,
                              MAT_PET_KEY_8,
                              MAT_PET_KEY_9,
                              MAT_PET_KEY_COLON,
                              MAT_PET_KEY_SEMICOLON,
                              MAT_PET_KEY_LESS_THAN,
                              MAT_PET_KEY_EQUALS,
                              MAT_PET_KEY_GREATER_THAN,
                              MAT_PET_KEY_QUESTION_MARK,
                              MAT_PET_KEY_AT,
                              MAT_PET_KEY_A,
                              MAT_PET_KEY_B,
                              MAT_PET_KEY_C,
                              MAT_PET_KEY_D,
                              MAT_PET_KEY_E,
                              MAT_PET_KEY_F,
                              MAT_PET_KEY_G,
                              MAT_PET_KEY_H,
                              MAT_PET_KEY_I,
                              MAT_PET_KEY_J,
                              MAT_PET_KEY_K,
                              MAT_PET_KEY_L,
                              MAT_PET_KEY_M,
                              MAT_PET_KEY_N,
                              MAT_PET_KEY_O,
                              MAT_PET_KEY_P,
                              MAT_PET_KEY_Q,
                              MAT_PET_KEY_R,
                              MAT_PET_KEY_S,
                              MAT_PET_KEY_T,
                              MAT_PET_KEY_U,
                              MAT_PET_KEY_V,
                              MAT_PET_KEY_W,
                              MAT_PET_KEY_X,
                              MAT_PET_KEY_Y,
                              MAT_PET_KEY_Z,
                              MAT_PET_KEY_LEFT_BRACKET,
                              MAT_PET_KEY_NONE, // backslash
                              MAT_PET_KEY_RIGHT_BRACKET,
                              MAT_PET_KEY_UP_ARROW,
                             };

#define ASCII_MAP_TBL_SZ    (sizeof(ascii_map)/sizeof(ascii_map[0]))

static uint8_t key_map[][5] = {
                               {' ', SCAN_C64_KEY_SPACE, MAT_PET_KEY_SPACE, MAT_PET_KEY_SPACE, MAT_PET_KEY_NONE},
                               {'*', SCAN_C64_KEY_ASTERIX, MAT_PET_KEY_ASTERIX, MAT_PET_KEY_AT, MAT_PET_KEY_LEFT_ARROW | MOD_OVERRIDE},
                               {'+', SCAN_C64_KEY_PLUS, MAT_PET_KEY_PLUS, MAT_PET_KEY_LEFT_BRACKET, MAT_PET_KEY_AMPERSAND | MOD_OVERRIDE},
                               {',', SCAN_C64_KEY_COMMA, MAT_PET_KEY_COMMA, MAT_PET_KEY_LESS_THAN | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'-', SCAN_C64_KEY_MINUS, MAT_PET_KEY_MINUS, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE},
                               {'.', SCAN_C64_KEY_PERIOD, MAT_PET_KEY_PERIOD, MAT_PET_KEY_GREATER_THAN | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'/', SCAN_C64_KEY_SLASH, MAT_PET_KEY_SLASH, MAT_PET_KEY_QUESTION_MARK | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'0', SCAN_C64_KEY_0, MAT_PET_KEY_0, MAT_PET_KEY_0, MAT_PET_KEY_REVERSE | MOD_OVERRIDE},
                               {'1', SCAN_C64_KEY_1, MAT_PET_KEY_1, MAT_PET_KEY_EXCLAMATION | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'2', SCAN_C64_KEY_2, MAT_PET_KEY_2, MAT_PET_KEY_DOUBLE_QUOTE | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'3', SCAN_C64_KEY_3, MAT_PET_KEY_3, MAT_PET_KEY_HASH | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'4', SCAN_C64_KEY_4, MAT_PET_KEY_4, MAT_PET_KEY_DOLLAR_SIGN | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'5', SCAN_C64_KEY_5, MAT_PET_KEY_5, MAT_PET_KEY_PERCENT | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'6', SCAN_C64_KEY_6, MAT_PET_KEY_6, MAT_PET_KEY_AMPERSAND | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'7', SCAN_C64_KEY_7, MAT_PET_KEY_7, MAT_PET_KEY_APOSTROPHE | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'8', SCAN_C64_KEY_8, MAT_PET_KEY_8, MAT_PET_KEY_LEFT_PAREN | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'9', SCAN_C64_KEY_9, MAT_PET_KEY_9, MAT_PET_KEY_RIGHT_PAREN | MOD_OVERRIDE, MAT_PET_KEY_REVERSE},
                               {':', SCAN_C64_KEY_COLON, MAT_PET_KEY_COLON, MAT_PET_KEY_LEFT_BRACKET | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {';', SCAN_C64_KEY_SEMICOLON, MAT_PET_KEY_SEMICOLON, MAT_PET_KEY_RIGHT_BRACKET | MOD_OVERRIDE, MAT_PET_KEY_NONE},
                               {'=', SCAN_C64_KEY_EQUALS, MAT_PET_KEY_EQUALS, MAT_PET_KEY_RIGHT_BRACKET, MAT_PET_KEY_BACKSLASH | MOD_OVERRIDE},
                               {'@', SCAN_C64_KEY_AT, MAT_PET_KEY_AT, MAT_PET_KEY_COLON, MAT_PET_KEY_DOLLAR_SIGN | MOD_OVERRIDE},
                               {'A', SCAN_C64_KEY_A, MAT_PET_KEY_A, MAT_PET_KEY_A, MAT_PET_KEY_0 | MOD_OVERRIDE},
                               {'B', SCAN_C64_KEY_B, MAT_PET_KEY_B, MAT_PET_KEY_B, MAT_PET_KEY_QUESTION_MARK | MOD_OVERRIDE},
                               {'C', SCAN_C64_KEY_C, MAT_PET_KEY_C, MAT_PET_KEY_C, MAT_PET_KEY_LESS_THAN | MOD_OVERRIDE},
                               {'D', SCAN_C64_KEY_D, MAT_PET_KEY_D, MAT_PET_KEY_D, MAT_PET_KEY_COMMA | MOD_OVERRIDE},
                               {'E', SCAN_C64_KEY_E, MAT_PET_KEY_E, MAT_PET_KEY_E, MAT_PET_KEY_1 | MOD_OVERRIDE},
                               {'F', SCAN_C64_KEY_F, MAT_PET_KEY_F, MAT_PET_KEY_F, MAT_PET_KEY_SEMICOLON | MOD_OVERRIDE},
                               {'G', SCAN_C64_KEY_G, MAT_PET_KEY_G, MAT_PET_KEY_G, MAT_PET_KEY_PERCENT | MOD_OVERRIDE},
                               {'H', SCAN_C64_KEY_H, MAT_PET_KEY_H, MAT_PET_KEY_H, MAT_PET_KEY_4 | MOD_OVERRIDE},
                               {'I', SCAN_C64_KEY_I, MAT_PET_KEY_I, MAT_PET_KEY_I, MAT_PET_KEY_DOUBLE_QUOTE | MOD_OVERRIDE},
                               {'J', SCAN_C64_KEY_J, MAT_PET_KEY_J, MAT_PET_KEY_J, MAT_PET_KEY_5 | MOD_OVERRIDE},
                               {'K', SCAN_C64_KEY_K, MAT_PET_KEY_K, MAT_PET_KEY_K, MAT_PET_KEY_EXCLAMATION | MOD_OVERRIDE},
                               {'L', SCAN_C64_KEY_L, MAT_PET_KEY_L, MAT_PET_KEY_L, MAT_PET_KEY_6 | MOD_OVERRIDE},
                               {'M', SCAN_C64_KEY_M, MAT_PET_KEY_M, MAT_PET_KEY_M, MAT_PET_KEY_APOSTROPHE | MOD_OVERRIDE},
                               {'N', SCAN_C64_KEY_N, MAT_PET_KEY_N, MAT_PET_KEY_N, MAT_PET_KEY_ASTERIX | MOD_OVERRIDE},
                               {'O', SCAN_C64_KEY_O, MAT_PET_KEY_O, MAT_PET_KEY_O, MAT_PET_KEY_9 | MOD_OVERRIDE},
                               {'P', SCAN_C64_KEY_P, MAT_PET_KEY_P, MAT_PET_KEY_P, MAT_PET_KEY_SLASH | MOD_OVERRIDE},
                               {'Q', SCAN_C64_KEY_Q, MAT_PET_KEY_Q, MAT_PET_KEY_Q, MAT_PET_KEY_PLUS | MOD_OVERRIDE},
                               {'R', SCAN_C64_KEY_R, MAT_PET_KEY_R, MAT_PET_KEY_R, MAT_PET_KEY_2 | MOD_OVERRIDE},
                               {'S', SCAN_C64_KEY_S, MAT_PET_KEY_S, MAT_PET_KEY_S, MAT_PET_KEY_PERIOD | MOD_OVERRIDE},
                               {'T', SCAN_C64_KEY_T, MAT_PET_KEY_T, MAT_PET_KEY_T, MAT_PET_KEY_HASH | MOD_OVERRIDE},
                               {'U', SCAN_C64_KEY_U, MAT_PET_KEY_U, MAT_PET_KEY_U, MAT_PET_KEY_8 | MOD_OVERRIDE},
                               {'V', SCAN_C64_KEY_V, MAT_PET_KEY_V, MAT_PET_KEY_V, MAT_PET_KEY_GREATER_THAN | MOD_OVERRIDE},
                               {'W', SCAN_C64_KEY_W, MAT_PET_KEY_W, MAT_PET_KEY_W, MAT_PET_KEY_3 | MOD_OVERRIDE},
                               {'X', SCAN_C64_KEY_X, MAT_PET_KEY_X, MAT_PET_KEY_X, MAT_PET_KEY_EQUALS | MOD_OVERRIDE},
                               {'Y', SCAN_C64_KEY_Y, MAT_PET_KEY_Y, MAT_PET_KEY_Y, MAT_PET_KEY_7 | MOD_OVERRIDE},
                               {'Z', SCAN_C64_KEY_Z, MAT_PET_KEY_Z, MAT_PET_KEY_Z, MAT_PET_KEY_MINUS | MOD_OVERRIDE},
                               {'\\', SCAN_C64_KEY_POUND, MAT_PET_KEY_BACKSLASH, MAT_PET_KEY_RIGHT_PAREN, MAT_PET_KEY_LEFT_PAREN | MOD_OVERRIDE},
                               {'^', SCAN_C64_KEY_UP_ARROW, MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_NONE}
                              };

#define MAP_TBL_SZ          (sizeof(key_map)/sizeof(key_map[0]))


void set_switch(uint8_t sw, uint8_t state) {
  debug_putc('(');
  if(state)
    debug_putc('+');
  else
    debug_putc('-');
  debug_puthex(sw & 15);
  debug_putc(':');
  debug_puthex(sw >> 4);
  debug_putc(')');
  xpt_send(sw,state);
}


void set_vkey(uint8_t unshifted, uint8_t shifted, uint8_t cmdr, uint8_t state) {
  // TODO optimize for MAT_PET_KEY_NONE

  switch(meta) {
    case META_LSHIFT:
    case META_RSHIFT:
    case (META_LSHIFT | META_RSHIFT):
      if(!state)
        set_switch(shifted & ~MOD_OVERRIDE, state);
      if(shifted & MOD_OVERRIDE) {        // shift on and virtual unshift needed
        if(meta & META_LSHIFT)           // do I need to fix left?
          set_switch(MAT_PET_KEY_LSHIFT, !state);
        if(meta & META_RSHIFT)           // do I need to fix right?
          set_switch(MAT_PET_KEY_RSHIFT, !state);
      }
      if(state)
        set_switch(shifted & ~MOD_OVERRIDE, state);
      break;
    case META_CMDR:
      if(!state)
        set_switch(cmdr & ~MOD_OVERRIDE, state);
      if(cmdr & MOD_OVERRIDE) {           // shift off and virtual shift needed
        set_switch(MAT_PET_KEY_LSHIFT, state);
      }
      if(state)
        set_switch(cmdr & ~MOD_OVERRIDE, state);
      break;
    case (META_CMDR | META_LSHIFT):
    case (META_CMDR | META_RSHIFT):
    case (META_CMDR | META_LSHIFT | META_RSHIFT):
      // do nothing
      break;
    default:
      // unshifted, no CMDR key
      if(!state)
        set_switch(unshifted & ~MOD_OVERRIDE, state);
      if(unshifted & MOD_OVERRIDE) {      // shift off and virtual shift needed
        set_switch(MAT_PET_KEY_LSHIFT, state);
      }
      if(state)
        set_switch(unshifted & ~MOD_OVERRIDE, state);
      break;
  }
}


static void set_ascii_vkey(char key) {
  uint8_t map = MAT_PET_KEY_NONE;

  switch(key) {
    default:
      if(key >= ' ' && key <= (ASCII_MAP_TBL_SZ + ' ')) {
        debug_putc(key);
        map = ascii_map[key - ' '];
      }
      break;
    case 13:
      debug_putc(13);
      map = MAT_PET_KEY_RETURN;
      break;
  }
  if(map != MAT_PET_KEY_NONE) {
    set_switch(map, TRUE);
    _delay_ms(16);
    set_switch(map, FALSE);
    _delay_ms(16);
  }
}


static void map_ascii_string(char *str) {
  char *p = &str[0];

  if(meta & META_LSHIFT)           // do I need to fix left?
    set_switch(MAT_PET_KEY_LSHIFT, FALSE);
  if(meta & META_RSHIFT)           // do I need to fix right?
    set_switch(MAT_PET_KEY_RSHIFT, FALSE);
  while(*p) {
    set_ascii_vkey(*p);
    p++;
  }
  if(meta & META_LSHIFT)           // do I need to fix left?
    set_switch(MAT_PET_KEY_LSHIFT, TRUE);
  if(meta & META_RSHIFT)           // do I need to fix right?
    set_switch(MAT_PET_KEY_RSHIFT, TRUE);
}


static void map_key(uint8_t key) {
  uint8_t state;
  uint8_t i;
  uint8_t cmp;

  cmp = key & KB_SCAN_CODE_MASK;
  state = (key & KB_KEY_UP ? FALSE : TRUE);


  //debug_puthex(key & KB_SCAN_CODE_MASK);
  switch(key & KB_SCAN_CODE_MASK) {
  default:
    for(i = 0; i < MAP_TBL_SZ; i++) {
      if(key_map[i][1] == cmp) {
        debug_putc(key_map[i][0]);
        set_vkey(key_map[i][2], key_map[i][3], key_map[i][4], state);
        break;
      }
    }
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
  case SCAN_C64_KEY_CRSR_DOWN:
    debug_puts("CSDN");
    set_vkey(MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_NONE, state);
    break;

  case SCAN_C64_KEY_HOME:
    debug_puts("HOME");
    set_vkey(MAT_PET_KEY_HOME, MAT_PET_KEY_HOME, MAT_PET_KEY_NONE, state);
    break;
  case SCAN_C64_KEY_LEFT_ARROW:
    debug_puts("LEFTARROW");
    set_vkey(MAT_PET_KEY_LEFT_ARROW, MAT_PET_KEY_NONE, MAT_PET_KEY_LEFT_ARROW, state);
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
    break;

  case SCAN_C64_KEY_F1:
    debug_puts("F1");
    if(state) {
      if(meta & META_SHIFT_MASK) {
        map_ascii_string("F2\r");
      } else {
        map_ascii_string("DIRECTORY\r");
      }
    }
    break;
  case SCAN_C64_KEY_F3:
    debug_puts("F3");
    if(state) {
      if(meta & META_SHIFT_MASK) {
        map_ascii_string("F4\r");
      } else {
        map_ascii_string("F3\r");
      }
    }
    break;
  case SCAN_C64_KEY_F5:
    debug_puts("F5");
    if(state) {
      if(meta & META_SHIFT_MASK) {
        map_ascii_string("F6\r");
      } else {
        map_ascii_string("F5\r");
      }
    }
    break;
  case SCAN_C64_KEY_F7:
    debug_puts("F7");
    if(state) {
      if(meta & META_SHIFT_MASK) {
        map_ascii_string("F8\r");
      } else {
        map_ascii_string("F7\r");
      }
    }
    break;
  }
}


void vkb_init(void) {
  kb_init();
  xpt_init();
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
