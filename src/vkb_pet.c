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
#include "kb_macro.h"
#include "uart.h"
#include "vkb.h"

#include "vkb_pet.h"

typedef enum {
  OPTST_IDLE = 0,
  OPTST_MAP_KEY,
  OPTST_MAP_KEY_DATA,
  OPTST_MAP_JOY,
  OPTST_JOYUP,
  OPTST_JOYDN,
  OPTST_JOYLT,
  OPTST_JOYRT,
  OPTST_JOYF1,
  OPTST_JOYF2,
  OPTST_DEBUG
} opstates_t;


static uint8_t _debug = FALSE;
static uint8_t _meta = 0;
//static uint8_t _config_meta = 0;
static uint8_t _shift_override_key = MAT_PET_KEY_NONE;
static uint8_t _config = FALSE;
//static uint8_t _config = TRUE;
static uint8_t _buf[64]; // TODO fix buffer overrun error.

static opstates_t _opt_state = OPTST_IDLE;
static uint8_t _opt_num;
static uint8_t _joy_keys[2][6];
static uint8_t _key;


void vkb_irq(void) {
  kb_scan();
  //DDRB |= _BV(PIN7);
  //PORTB ^= _BV(PIN7);
}

#define META_FLAG_LSHIFT    0x01
#define META_FLAG_RSHIFT    0x02
#define META_FLAG_CTRL      0x04
#define META_FLAG_CBM       0x08
#define META_SHIFT_MASK     (META_FLAG_LSHIFT | META_FLAG_RSHIFT)

#define IS_SHIFTED()        (_meta & META_SHIFT_MASK)
#define DELAY_JIFFY()       _delay_ms(1000/50)


#define SW_SHIFT_OVERRIDE   0x80
#define SW_VALUE_MASK       (uint8_t)~SW_SHIFT_OVERRIDE

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
                               {'*', SCAN_C64_KEY_ASTERIX, MAT_PET_KEY_ASTERIX, MAT_PET_KEY_AT, MAT_PET_KEY_LEFT_ARROW | SW_SHIFT_OVERRIDE},
                               {'+', SCAN_C64_KEY_PLUS, MAT_PET_KEY_PLUS, MAT_PET_KEY_LEFT_BRACKET, MAT_PET_KEY_AMPERSAND | SW_SHIFT_OVERRIDE},
                               {',', SCAN_C64_KEY_COMMA, MAT_PET_KEY_COMMA, MAT_PET_KEY_LESS_THAN | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'-', SCAN_C64_KEY_MINUS, MAT_PET_KEY_MINUS, MAT_PET_KEY_RIGHT_BRACKET, MAT_PET_KEY_BACKSLASH | SW_SHIFT_OVERRIDE},
                               {'.', SCAN_C64_KEY_PERIOD, MAT_PET_KEY_PERIOD, MAT_PET_KEY_GREATER_THAN | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'/', SCAN_C64_KEY_SLASH, MAT_PET_KEY_SLASH, MAT_PET_KEY_QUESTION_MARK | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'0', SCAN_C64_KEY_0, MAT_PET_KEY_0, MAT_PET_KEY_0, MAT_PET_KEY_REVERSE | SW_SHIFT_OVERRIDE},
                               {'1', SCAN_C64_KEY_1, MAT_PET_KEY_1, MAT_PET_KEY_EXCLAMATION | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'2', SCAN_C64_KEY_2, MAT_PET_KEY_2, MAT_PET_KEY_DOUBLE_QUOTE | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'3', SCAN_C64_KEY_3, MAT_PET_KEY_3, MAT_PET_KEY_HASH | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'4', SCAN_C64_KEY_4, MAT_PET_KEY_4, MAT_PET_KEY_DOLLAR_SIGN | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'5', SCAN_C64_KEY_5, MAT_PET_KEY_5, MAT_PET_KEY_PERCENT | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'6', SCAN_C64_KEY_6, MAT_PET_KEY_6, MAT_PET_KEY_AMPERSAND | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'7', SCAN_C64_KEY_7, MAT_PET_KEY_7, MAT_PET_KEY_APOSTROPHE | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'8', SCAN_C64_KEY_8, MAT_PET_KEY_8, MAT_PET_KEY_LEFT_PAREN | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'9', SCAN_C64_KEY_9, MAT_PET_KEY_9, MAT_PET_KEY_RIGHT_PAREN | SW_SHIFT_OVERRIDE, MAT_PET_KEY_REVERSE},
                               {':', SCAN_C64_KEY_COLON, MAT_PET_KEY_COLON, MAT_PET_KEY_LEFT_BRACKET | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {';', SCAN_C64_KEY_SEMICOLON, MAT_PET_KEY_SEMICOLON, MAT_PET_KEY_RIGHT_BRACKET | SW_SHIFT_OVERRIDE, MAT_PET_KEY_NONE},
                               {'=', SCAN_C64_KEY_EQUALS, MAT_PET_KEY_EQUALS, MAT_PET_KEY_NONE, MAT_PET_KEY_NONE},
                               {'@', SCAN_C64_KEY_AT, MAT_PET_KEY_AT, MAT_PET_KEY_COLON, MAT_PET_KEY_DOLLAR_SIGN | SW_SHIFT_OVERRIDE},
                               {'A', SCAN_C64_KEY_A, MAT_PET_KEY_A, MAT_PET_KEY_A, MAT_PET_KEY_0 | SW_SHIFT_OVERRIDE},
                               {'B', SCAN_C64_KEY_B, MAT_PET_KEY_B, MAT_PET_KEY_B, MAT_PET_KEY_QUESTION_MARK | SW_SHIFT_OVERRIDE},
                               {'C', SCAN_C64_KEY_C, MAT_PET_KEY_C, MAT_PET_KEY_C, MAT_PET_KEY_LESS_THAN | SW_SHIFT_OVERRIDE},
                               {'D', SCAN_C64_KEY_D, MAT_PET_KEY_D, MAT_PET_KEY_D, MAT_PET_KEY_COMMA | SW_SHIFT_OVERRIDE},
                               {'E', SCAN_C64_KEY_E, MAT_PET_KEY_E, MAT_PET_KEY_E, MAT_PET_KEY_1 | SW_SHIFT_OVERRIDE},
                               {'F', SCAN_C64_KEY_F, MAT_PET_KEY_F, MAT_PET_KEY_F, MAT_PET_KEY_SEMICOLON | SW_SHIFT_OVERRIDE},
                               {'G', SCAN_C64_KEY_G, MAT_PET_KEY_G, MAT_PET_KEY_G, MAT_PET_KEY_PERCENT | SW_SHIFT_OVERRIDE},
                               {'H', SCAN_C64_KEY_H, MAT_PET_KEY_H, MAT_PET_KEY_H, MAT_PET_KEY_4 | SW_SHIFT_OVERRIDE},
                               {'I', SCAN_C64_KEY_I, MAT_PET_KEY_I, MAT_PET_KEY_I, MAT_PET_KEY_DOUBLE_QUOTE | SW_SHIFT_OVERRIDE},
                               {'J', SCAN_C64_KEY_J, MAT_PET_KEY_J, MAT_PET_KEY_J, MAT_PET_KEY_5 | SW_SHIFT_OVERRIDE},
                               {'K', SCAN_C64_KEY_K, MAT_PET_KEY_K, MAT_PET_KEY_K, MAT_PET_KEY_EXCLAMATION | SW_SHIFT_OVERRIDE},
                               {'L', SCAN_C64_KEY_L, MAT_PET_KEY_L, MAT_PET_KEY_L, MAT_PET_KEY_6 | SW_SHIFT_OVERRIDE},
                               {'M', SCAN_C64_KEY_M, MAT_PET_KEY_M, MAT_PET_KEY_M, MAT_PET_KEY_APOSTROPHE | SW_SHIFT_OVERRIDE},
                               {'N', SCAN_C64_KEY_N, MAT_PET_KEY_N, MAT_PET_KEY_N, MAT_PET_KEY_ASTERIX | SW_SHIFT_OVERRIDE},
                               {'O', SCAN_C64_KEY_O, MAT_PET_KEY_O, MAT_PET_KEY_O, MAT_PET_KEY_9 | SW_SHIFT_OVERRIDE},
                               {'P', SCAN_C64_KEY_P, MAT_PET_KEY_P, MAT_PET_KEY_P, MAT_PET_KEY_SLASH | SW_SHIFT_OVERRIDE},
                               {'Q', SCAN_C64_KEY_Q, MAT_PET_KEY_Q, MAT_PET_KEY_Q, MAT_PET_KEY_PLUS | SW_SHIFT_OVERRIDE},
                               {'R', SCAN_C64_KEY_R, MAT_PET_KEY_R, MAT_PET_KEY_R, MAT_PET_KEY_2 | SW_SHIFT_OVERRIDE},
                               {'S', SCAN_C64_KEY_S, MAT_PET_KEY_S, MAT_PET_KEY_S, MAT_PET_KEY_PERIOD | SW_SHIFT_OVERRIDE},
                               {'T', SCAN_C64_KEY_T, MAT_PET_KEY_T, MAT_PET_KEY_T, MAT_PET_KEY_HASH | SW_SHIFT_OVERRIDE},
                               {'U', SCAN_C64_KEY_U, MAT_PET_KEY_U, MAT_PET_KEY_U, MAT_PET_KEY_8 | SW_SHIFT_OVERRIDE},
                               {'V', SCAN_C64_KEY_V, MAT_PET_KEY_V, MAT_PET_KEY_V, MAT_PET_KEY_GREATER_THAN | SW_SHIFT_OVERRIDE},
                               {'W', SCAN_C64_KEY_W, MAT_PET_KEY_W, MAT_PET_KEY_W, MAT_PET_KEY_3 | SW_SHIFT_OVERRIDE},
                               {'X', SCAN_C64_KEY_X, MAT_PET_KEY_X, MAT_PET_KEY_X, MAT_PET_KEY_EQUALS | SW_SHIFT_OVERRIDE},
                               {'Y', SCAN_C64_KEY_Y, MAT_PET_KEY_Y, MAT_PET_KEY_Y, MAT_PET_KEY_7 | SW_SHIFT_OVERRIDE},
                               {'Z', SCAN_C64_KEY_Z, MAT_PET_KEY_Z, MAT_PET_KEY_Z, MAT_PET_KEY_MINUS | SW_SHIFT_OVERRIDE},
                               {'\\', SCAN_C64_KEY_POUND, MAT_PET_KEY_BACKSLASH, MAT_PET_KEY_RIGHT_PAREN, MAT_PET_KEY_LEFT_PAREN | SW_SHIFT_OVERRIDE},
                               {'^', SCAN_C64_KEY_UP_ARROW, MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_UP_ARROW, MAT_PET_KEY_NONE}
                              };

#define MAP_TBL_SZ          (sizeof(key_map)/sizeof(key_map[0]))


void debug_putkey(uint8_t sw, uint8_t state) {
  if(_debug) {
    debug_putc('(');
    if(state)
      debug_putc('+');
    else
      debug_putc('-');
    debug_puthex(sw & 15);
    debug_putc(':');
    debug_puthex(sw >> 4);
    debug_putc(')');
  }
}


void set_switch(uint8_t sw, uint8_t state) {
  debug_putkey(sw, state);
  xpt_send(sw,state);
}


static uint8_t set_vkey(uint8_t unshifted, uint8_t shifted, uint8_t cmdr, uint8_t state) {
  uint8_t vkey = MAT_PET_KEY_NONE;
  // TODO optimize for MAT_PET_KEY_NONE

  if(_shift_override_key != MAT_PET_KEY_NONE) {
    set_switch(_shift_override_key & ~SW_SHIFT_OVERRIDE, FALSE);
    _shift_override_key = MAT_PET_KEY_NONE;
  }
  switch(_meta) {
    case META_FLAG_LSHIFT:
    case META_FLAG_RSHIFT:
    case (META_FLAG_LSHIFT | META_FLAG_RSHIFT):
      if(!state)
        set_switch(shifted & ~SW_SHIFT_OVERRIDE, state);
      if(shifted & SW_SHIFT_OVERRIDE) {        // shift on and virtual unshift needed
        if(_meta & META_FLAG_LSHIFT)           // do I need to fix left?
          set_switch(MAT_PET_KEY_LSHIFT, !state);
        if(_meta & META_FLAG_RSHIFT)           // do I need to fix right?
          set_switch(MAT_PET_KEY_RSHIFT, !state);
        _shift_override_key = shifted;
        DELAY_JIFFY();
      }
      if(state)
        set_switch(shifted & ~SW_SHIFT_OVERRIDE, state);
      vkey = shifted;
      break;
    case META_FLAG_CBM:
      if(!state)
        set_switch(cmdr & ~SW_SHIFT_OVERRIDE, state);
      if(cmdr & SW_SHIFT_OVERRIDE) {           // shift off and virtual shift needed
        set_switch(MAT_PET_KEY_LSHIFT, state);
        _shift_override_key = cmdr;
      }
      if(state)
        set_switch(cmdr & ~SW_SHIFT_OVERRIDE, state);
      vkey = cmdr;
      break;
    case (META_FLAG_CBM | META_FLAG_LSHIFT):
    case (META_FLAG_CBM | META_FLAG_RSHIFT):
    case (META_FLAG_CBM | META_FLAG_LSHIFT | META_FLAG_RSHIFT):
      // do nothing
      break;
    default:
      // unshifted, no CMDR key
      if(!state)
        set_switch(unshifted & ~SW_SHIFT_OVERRIDE, state);
      if(unshifted & SW_SHIFT_OVERRIDE) {      // shift off and virtual shift needed
        set_switch(MAT_PET_KEY_LSHIFT, state);
        _shift_override_key = unshifted;
      }
      if(state)
        set_switch(unshifted & ~SW_SHIFT_OVERRIDE, state);
      vkey = unshifted;
      break;
  }
  // shifted or unshifted key with flags on current shift state;
  vkey = (vkey
         & ~SW_SHIFT_OVERRIDE) | (IS_SHIFTED() ? SW_SHIFT_OVERRIDE : 0);
  return vkey;
}


static void map_ascii_key(char key) {
  uint8_t map = MAT_PET_KEY_NONE;
  uint8_t pshift = FALSE;
  uint8_t pkey = key;

  if((key > '@') && (key < '[')) { // unshifted ASCII, shifted PETSCII
    // put shift on the lower values
    pshift = TRUE;
  } else if ((key > '`') && (key < '{')) { // shifted ASCII, unshifted PETSCII
    pkey = key & ~0x20;
  }

  switch(pkey) {
    default:
      if(pkey >= ' ' && pkey <= (ASCII_MAP_TBL_SZ + ' ')) {
        debug_putc(key);
        map = ascii_map[pkey - ' '];
      }
      break;
    case 13:
      debug_putc(13);
      // send Shift Return in config mode
      map = MAT_PET_KEY_RETURN;
      pshift = _config;
      break;
  }
  if(map != MAT_PET_KEY_NONE) {
    if(pshift)
      set_switch(MAT_PET_KEY_LSHIFT, TRUE);
    set_switch(map, TRUE);
    DELAY_JIFFY();
    set_switch(map, FALSE);
    if(pshift)
      set_switch(MAT_PET_KEY_LSHIFT, FALSE);
    DELAY_JIFFY();
  }
}


static void map_ascii_string(char *str) {
  char *p = &str[0];

  if(_meta & META_FLAG_LSHIFT)           // do I need to fix left?
    set_switch(MAT_PET_KEY_LSHIFT, FALSE);
  if(_meta & META_FLAG_RSHIFT)           // do I need to fix right?
    set_switch(MAT_PET_KEY_RSHIFT, FALSE);
  while(*p) {
    map_ascii_key(*p);
    p++;
  }
  if(_meta & META_FLAG_LSHIFT)           // do I need to fix left?
    set_switch(MAT_PET_KEY_LSHIFT, TRUE);
  if(_meta & META_FLAG_RSHIFT)           // do I need to fix right?
    set_switch(MAT_PET_KEY_RSHIFT, TRUE);
}


static void map_meta_key(uint8_t key, uint8_t state) {
  switch(key) {
    case SCAN_C64_KEY_LSHIFT:
      debug_puts("LSHIFT");
      _meta = (_meta & ~META_FLAG_LSHIFT) | (state ? META_FLAG_LSHIFT: 0);
      set_vkey(MAT_PET_KEY_LSHIFT, MAT_PET_KEY_LSHIFT, MAT_PET_KEY_LSHIFT, state);
      break;
    case SCAN_C64_KEY_RSHIFT:
      debug_puts("RSHIFT");
      _meta = (_meta & ~META_FLAG_RSHIFT) | (state ? META_FLAG_RSHIFT: 0);
      set_vkey(MAT_PET_KEY_RSHIFT, MAT_PET_KEY_RSHIFT, MAT_PET_KEY_RSHIFT, state);
      break;

    case SCAN_C64_KEY_CBM:
      debug_puts("CBM");
      // no key to depress
      _meta = (_meta & ~META_FLAG_CBM) | (state ? META_FLAG_CBM: 0);
      break;
    case SCAN_C64_KEY_CTRL:
      debug_puts("CTRL");
      _meta = (_meta & ~META_FLAG_CTRL) | (state ? META_FLAG_CTRL: 0);
      break;
  }
}

static void map_function_key(char *unshifted, char *shifted) {
  if(!_config) { // don't send in config mode
    if(_meta & META_SHIFT_MASK) {
      map_ascii_string(shifted);
    } else {
      map_ascii_string(unshifted);
    }
  }
}

static uint8_t  map_macro(uint8_t key, uint8_t state) {
  uint8_t i;
  uint8_t len;
  uint8_t override;
  uint8_t map;

  //debug_putc('m');
  //debug_puthex(key | (IS_SHIFTED() ? SW_SHIFT_OVERRIDE : 0));
  if(kbm_find(key | (IS_SHIFTED() ? SW_SHIFT_OVERRIDE : 0), &len, _buf)
                 == KBMRES_SUCCESS) {
    debug_putc(state ? '+' : '-');
    debug_puts("macro");
    if(state) {  // only handle macros on key down.
      if(_meta & META_FLAG_LSHIFT)
        set_switch(MAT_PET_KEY_LSHIFT, FALSE);
      if(_meta & META_FLAG_RSHIFT)
        set_switch(MAT_PET_KEY_RSHIFT, FALSE);

      _debug = TRUE;
      for(i = 0; i < len; i++) {
        override = _buf[i] & SW_SHIFT_OVERRIDE;
        map = _buf[i] & ~SW_SHIFT_OVERRIDE;

        if(map != MAT_PET_KEY_NONE) {
          if(override)
            set_switch(MAT_PET_KEY_LSHIFT, TRUE);
          set_switch(map, TRUE);
          DELAY_JIFFY();
          set_switch(map, FALSE);
          if(override)
            set_switch(MAT_PET_KEY_LSHIFT, FALSE);
          DELAY_JIFFY();
        }
      }
      _debug = FALSE;

      if(_meta & META_FLAG_LSHIFT)
        set_switch(MAT_PET_KEY_LSHIFT, TRUE);
      if(_meta & META_FLAG_RSHIFT)
        set_switch(MAT_PET_KEY_RSHIFT, TRUE);
    }
    return TRUE;
  }
  return FALSE;
}


static uint8_t map_key(uint8_t key) {
  uint8_t state;
  uint8_t i;
  uint8_t cmp;
  uint8_t mapped = MAT_PET_KEY_NONE;

  cmp = key & KB_SCAN_CODE_MASK;
  state = (key & KB_KEY_UP ? FALSE : TRUE);

  if((cmp == SCAN_C64_KEY_DELETE)
     && (_meta & META_FLAG_CTRL)
     && (_meta & META_FLAG_CBM)
    ) {
    if(!state) { // enter CONFIG mode on key up.
      map_ascii_string("config mode on\r");
      _config = !_config;
    }
  } else {
    map_meta_key(cmp, state); // map meta keys
    if(_config || (!_config && !map_macro(cmp, state))) {
      switch(cmp) {
      default:
        for(i = 0; i < MAP_TBL_SZ; i++) {
          if(key_map[i][1] == cmp) {
            debug_putc(key_map[i][0]);
            mapped = set_vkey(key_map[i][2], key_map[i][3], key_map[i][4], state);
            if(_config)
              debug_putkey(mapped & SW_VALUE_MASK, state);
            break;
          }
        }
        break;
      case SCAN_C64_KEY_DELETE:
        debug_puts("DELETE");
        mapped = set_vkey(MAT_PET_KEY_DELETE, MAT_PET_KEY_DELETE, MAT_PET_KEY_NONE, state);
        break;
      case SCAN_C64_KEY_RETURN:
        debug_puts("RETURN");
        // add shift when in CONFIG mode.
        mapped = set_vkey(MAT_PET_KEY_RETURN | (_config ? SW_SHIFT_OVERRIDE : 0),
                          MAT_PET_KEY_RETURN,
                          MAT_PET_KEY_NONE,
                          state
                         );
        break;
      case SCAN_C64_KEY_CRSR_RIGHT:
        debug_puts("CSRT");
        mapped = set_vkey(MAT_PET_KEY_CRSR_RIGHT, MAT_PET_KEY_CRSR_RIGHT, MAT_PET_KEY_NONE, state);
        break;
      case SCAN_C64_KEY_CRSR_DOWN:
        debug_puts("CSDN");
        mapped = set_vkey(MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_CRSR_DOWN, MAT_PET_KEY_NONE, state);
        break;

      case SCAN_C64_KEY_HOME:
        debug_puts("HOME");
        mapped = set_vkey(MAT_PET_KEY_HOME, MAT_PET_KEY_HOME, MAT_PET_KEY_NONE, state);
        break;
      case SCAN_C64_KEY_LEFT_ARROW:
        debug_puts("LEFTARROW");
        mapped = set_vkey(MAT_PET_KEY_LEFT_ARROW, MAT_PET_KEY_NONE, MAT_PET_KEY_LEFT_ARROW, state);
        break;
      case SCAN_C64_KEY_RUN_STOP:
        debug_puts("RUN/STOP");
        mapped = set_vkey(MAT_PET_KEY_RUN_STOP, MAT_PET_KEY_RUN_STOP, MAT_PET_KEY_NONE, state);
        break;

      case SCAN_C64_KEY_F1:
        mapped = MAT_PET_KEY_NONE;
        debug_puts("F1");
        if(state) {
          map_function_key("directory\r", "f2\r");
        }
        break;
      case SCAN_C64_KEY_F3:
        mapped = MAT_PET_KEY_NONE;
        debug_puts("F3");
        if(state) {
          map_function_key("dload \"*\"\r", "f4\r");
        }
        break;
      case SCAN_C64_KEY_F5:
        mapped = MAT_PET_KEY_NONE;
        debug_puts("F5");
        if(state) {
          map_function_key("f5\r", "f6\r");
        }
        break;
      case SCAN_C64_KEY_F7:
        mapped = MAT_PET_KEY_NONE;
        debug_puts("F7");
        if(state) {
          map_function_key("f7\r", "f8\r");
        }
        break;
      }
    }
  }
  return mapped;
}


void map_option(uint8_t key) {
  uint8_t state;
  uint8_t cmp;
  uint8_t vkey;

  cmp = key & KB_SCAN_CODE_MASK;
  state = (key & KB_KEY_UP ? FALSE : TRUE);
  //override = (_meta & META_SHIFT_MASK ? SW_SHIFT_OVERRIDE : 0);

  map_meta_key(cmp, state); // handle meta keys
  if(!state && (_meta & META_FLAG_CTRL) && (_meta & META_FLAG_CBM) && (cmp == SCAN_C64_KEY_DELETE)) {
    map_ascii_string("config mode off\n");
    _config = !_config;
    // TODO save state, most likely
  } else {
    if(!state) {
      switch(_opt_state) {
        case OPTST_IDLE:
          _key = cmp | (IS_SHIFTED() ? SW_SHIFT_OVERRIDE : 0); // save off the key
          switch(cmp) {
            case SCAN_C64_KEY_J:
              _opt_state = OPTST_MAP_JOY;
              map_ascii_string("map joystick. joy#");
              break;
            case SCAN_C64_KEY_F1:
              _opt_state = OPTST_MAP_KEY_DATA;
              _opt_num = 0;
              map_ascii_string("map function key (SH-RETURN to finish) #");
              map_ascii_key(IS_SHIFTED() ? '2' : '1');
              map_ascii_key(':');
              break;
            case SCAN_C64_KEY_F3:
              _opt_state = OPTST_MAP_KEY_DATA;
              _opt_num = 0;
              map_ascii_string("map function key (SH-RETURN to finish) #");
              map_ascii_key(IS_SHIFTED() ? '4' : '3');
              map_ascii_key(':');
              break;
            case SCAN_C64_KEY_F5:
              _opt_state = OPTST_MAP_KEY_DATA;
              _opt_num = 0;
              map_ascii_string("map function key (SH-RETURN to finish) #");
              map_ascii_key(IS_SHIFTED() ? '6' : '5');
              map_ascii_key(':');
              break;
            case SCAN_C64_KEY_F7:
              _opt_state = OPTST_MAP_KEY_DATA;
              _opt_num = 0;
              map_ascii_string("map function key (SH-RETURN to finish) #");
              map_ascii_key(IS_SHIFTED() ? '8' : '7');
              map_ascii_key(':');
              break;
            case SCAN_C64_KEY_M: // map a key
              _opt_state = OPTST_MAP_KEY;
              map_ascii_string("map which key?:");
              break;
          }
          break;
        case OPTST_MAP_KEY:
          switch(cmp) {
            case SCAN_C64_KEY_LSHIFT:
            case SCAN_C64_KEY_RSHIFT:
            case SCAN_C64_KEY_CBM:
            case SCAN_C64_KEY_CTRL:
              // ignore.
              map_ascii_string("invalid\r");
              _opt_state = OPTST_IDLE;
              break;
            default:
              _key = cmp | (IS_SHIFTED() ? SW_SHIFT_OVERRIDE : 0); // save off the key
              _opt_state = OPTST_MAP_KEY_DATA;
              _opt_num = 0;
              map_key(key);
              map_ascii_string(" (SH-RETURN to finish):");
              break;
          }
          break;
        case OPTST_DEBUG:
          break;
        case OPTST_MAP_JOY:
          switch(cmp) {
            case SCAN_C64_KEY_1:
              map_ascii_string("1.up:");
              _opt_state = OPTST_JOYUP;
              _opt_num = 0;
              break;
            case SCAN_C64_KEY_2:
              map_ascii_string("2.up:");
              _opt_state = OPTST_JOYUP;
              _opt_num = 1;
              break;
            default:
              map_ascii_string(".INVALID\r");
              _opt_state = OPTST_IDLE;
              break;
          }
          break;
        case OPTST_JOYUP:
          map_key(cmp);
          _joy_keys[_opt_num][JOY_UP] = cmp;
          map_ascii_string(".down:");
          _opt_state = OPTST_JOYDN;
          break;
        case OPTST_JOYDN:
          map_key(cmp);
          _joy_keys[_opt_num][JOY_DN] = cmp;
          map_ascii_string(".left:");
          _opt_state = OPTST_JOYLT;
          break;
        case OPTST_JOYLT:
          map_key(cmp);
          _joy_keys[_opt_num][JOY_LT] = cmp;
          map_ascii_string(".right:");
          _opt_state = OPTST_JOYRT;
          break;
        case OPTST_JOYRT:
          map_key(cmp);
          _joy_keys[_opt_num][JOY_RT] = cmp;
          map_ascii_string(".fire 1:");
          _opt_state = OPTST_JOYF1;
          break;
        case OPTST_JOYF1:
          map_key(cmp);
          _joy_keys[_opt_num][JOY_F1] = cmp;
          map_ascii_string(".fire 2:");
          _opt_state = OPTST_JOYF2;
          break;
        case OPTST_JOYF2:
          map_key(cmp);
          _joy_keys[_opt_num][JOY_F2] = cmp;
          map_ascii_string(".mapped\r");
          _opt_state = OPTST_IDLE;
          break;
        default:
          break;
      }
    }
    switch(_opt_state) {
      case OPTST_MAP_KEY_DATA:
        if(state && IS_SHIFTED() && (cmp == SCAN_C64_KEY_RETURN)) { // End Function
          // shift return ends definition
          _opt_state = OPTST_IDLE;
          kbm_del(_key); // delete old mapping;
          //debug_trace(_buf,0,_opt_num);
          //debug_puthex(_key);
          kbm_add(_key, _opt_num, _buf);
          map_ascii_string("mapped\r");
        } else {
          switch(cmp) {
            case SCAN_C64_KEY_LSHIFT:
            case SCAN_C64_KEY_RSHIFT:
            case SCAN_C64_KEY_CBM:
            case SCAN_C64_KEY_CTRL:
              // ignore.
              break;
            default:
              //_debug = TRUE;
              vkey = map_key(key);
              debug_putkey(vkey, state);
              //_debug = FALSE;
              if(state) // if key down.
                _buf[_opt_num++] = vkey;
              break;
          }
        }
        break;
      default:
        break;
    }
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
      if(_config)
        map_option(key);
      else
        map_key(key);
      //debug_puthex(key>>3);
      //debug_putc('|');
      //debug_puthex(key & 0x07);
      //debug_putc('-');
    }
  }
}
