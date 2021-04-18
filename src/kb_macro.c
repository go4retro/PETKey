#include "config.h"
#include "debug.h"

#include "kb_macro.h"

uint16_t _len;
uint8_t _macros[MACRO_SZ];

static kbm_results_t find(uint8_t key, uint16_t *pos) {
  uint16_t i = 0;

  while(i < _len) {
    if(_macros[i] == key) {
      *pos = i;
      return KBMRES_SUCCESS;
    } else {
      i = i + _macros[i + 1] + 2;
    }
  }
  return KBMRES_NOT_FOUND;
}


kbm_results_t kbm_add(uint8_t key, uint8_t len, uint8_t *buf) {

  if(len + 2 + _len > MACRO_SZ)
    return KBMRES_TOO_LARGE;
  _macros[_len++] = key;
  _macros[_len++] = len;
  //debug_putc('A');
  for(uint8_t i = 0; i < len; i++) {
    _macros[_len++] = buf[i];
  //  debug_puthex(val[i]);
  }
  return KBMRES_SUCCESS;
}


kbm_results_t kbm_del(uint8_t key) {
  uint16_t i;
  uint8_t len;

  if(find(key, &i) == KBMRES_SUCCESS) {
    len = _macros[i + 1];
    for(uint8_t j = 0; j < len; j++) {
      _macros[i] = _macros[i + len + 2];
      i++;
    }
    _len = i;
    return KBMRES_SUCCESS;
  }
  return KBMRES_NOT_FOUND;
}


kbm_results_t kbm_find(uint8_t key, uint8_t *len, uint8_t *buf) {
  uint16_t i;

  if(find(key, &i) == KBMRES_SUCCESS) {
    *len = _macros[i + 1];
    //debug_putc('F');
    i+= 2;
    for(uint8_t j = 0; j < *len; j++) {
      buf[j] = _macros[i++];
      //debug_puthex(buf[j]);
    }
    return KBMRES_SUCCESS;
  }
  return KBMRES_NOT_FOUND;
}


void kbm_init(void) {
  _len = 0;
}
