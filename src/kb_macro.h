/*
 * kb_macro.h
 *
 *  Created on: Apr 16, 2021
 *      Author: brain
 */

#ifndef SRC_KB_MACRO_H
#define SRC_KB_MACRO_H

//#ifdef __AVR_ATmega162__ ||
#define MACRO_SZ 500

typedef enum {
  KBMRES_SUCCESS = 0,
  KBMRES_NOT_FOUND,
  KBMRES_TOO_LARGE
} kbm_results_t;

kbm_results_t kbm_add(uint8_t key, uint8_t len, uint8_t *val);
kbm_results_t kbm_del(uint8_t key);
kbm_results_t kbm_find(uint8_t key, uint8_t *len, uint8_t *val);
void kbm_init(void);

#endif /* SRC_KB_MACRO_H */
