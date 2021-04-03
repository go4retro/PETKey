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
 *  vkb_pet.h: Definitions for VIC/64 to PET mapping
 */

#ifndef VKB_PET_H
#define VKB_PET_H

/*
  X0 = 0
  X1 = 1
  X2 = 2
  X3 = 3
 */

#define MATRIX_X(x)           (  ((x & 1) ? 1 : 0)   \
                               | ((x & 2) ? 2 : 0) \
                               | ((x & 4) ? 4 : 0)  \
                               | ((x & 8) ? 8 : 0))

/*
  Y0 = 4
  Y1 = 5
  Y2 = 6
 */

#define MATRIX_Y(y)           (  ((y & 1) ? 16 : 0) \
                               | ((y & 2) ? 32 : 0) \
                               | ((y & 4) ? 64 : 0))


/*
 * From: http://www.6502.org/users/sjgray/projects/petkeyboard/index.html
 * PET Internal:
 * 12  11  10  9   8   7   6   5   4   3   2   1   J   H   F   E   D   C   B   A   <-LABEL ON PCB
 * 001 002 003 004 005 006 007 008 009 010 011 012 013 014 015 016 017 018 019 020 <-PIN
 * GND KEY R9  R8  R7  R6  R5  R4  R3  R2  R1  R0  C7  C6  C5  C4  C3  C2  C1  C0  <-FUNCTION
 */



#define MATRIX_MAP(x,y)         ( \
                                 MATRIX_X( \
                                          ( \
                                           x == 0 ? 13 : \
                                           x == 1 ? 12 : \
                                           x == 2 ? 11 : \
                                           x == 3 ? 10 : \
                                           x == 4 ? 9 : \
                                           x == 5 ? 8 : \
                                           x == 6 ? 4 : \
                                           x == 7 ? 3 : \
                                           x == 8 ? 2 : 1 \
                                          ) \
                                         ) \
                                 | \
                                 MATRIX_Y( \
                                          ( \
                                           y == 0 ? 6 :   \
                                           y == 1 ? 4 :   \
                                           y == 2 ? 7 :   \
                                           y == 3 ? 5 :   \
                                           y == 4 ? 0 :   \
                                           y == 5 ? 1 :   \
                                           y == 6 ? 2 : 3 \
                                          ) \
                                         ) \
                                )
                                


#define MAT_PET_KEY_EQUALS        MATRIX_MAP(0,0)
#define MAT_PET_KEY_PERIOD        MATRIX_MAP(0,1)

#define MAT_PET_KEY_NONE          MATRIX_MAP(0,2)

#define MAT_PET_KEY_RUN_STOP      MATRIX_MAP(0,3)
#define MAT_PET_KEY_LESS_THAN     MATRIX_MAP(0,4)
#define MAT_PET_KEY_SPACE         MATRIX_MAP(0,5)
#define MAT_PET_KEY_LEFT_BRACKET  MATRIX_MAP(0,6)
#define MAT_PET_KEY_REVERSE       MATRIX_MAP(0,7)

#define MAT_PET_KEY_MINUS         MATRIX_MAP(1,0)
#define MAT_PET_KEY_0             MATRIX_MAP(1,1)
#define MAT_PET_KEY_RSHIFT        MATRIX_MAP(1,2)
#define MAT_PET_KEY_GREATER_THAN  MATRIX_MAP(1,3)

#define MAT_PET_KEY_RIGHT_BRACKET MATRIX_MAP(1,5)
#define MAT_PET_KEY_AT            MATRIX_MAP(1,6)
#define MAT_PET_KEY_LSHIFT        MATRIX_MAP(1,7)

#define MAT_PET_KEY_PLUS          MATRIX_MAP(2,0)
#define MAT_PET_KEY_2             MATRIX_MAP(2,1)

#define MAT_PET_KEY_QUESTION_MARK MATRIX_MAP(2,3)
#define MAT_PET_KEY_COMMA         MATRIX_MAP(2,4)
#define MAT_PET_KEY_N             MATRIX_MAP(2,5)
#define MAT_PET_KEY_V             MATRIX_MAP(2,6)
#define MAT_PET_KEY_X             MATRIX_MAP(2,7)

#define MAT_PET_KEY_3             MATRIX_MAP(3,0)
#define MAT_PET_KEY_1             MATRIX_MAP(3,1)
#define MAT_PET_KEY_RETURN        MATRIX_MAP(3,2)
#define MAT_PET_KEY_SEMICOLON     MATRIX_MAP(3,3)
#define MAT_PET_KEY_M             MATRIX_MAP(3,4)
#define MAT_PET_KEY_B             MATRIX_MAP(3,5)
#define MAT_PET_KEY_C             MATRIX_MAP(3,6)
#define MAT_PET_KEY_Z             MATRIX_MAP(3,7)

#define MAT_PET_KEY_ASTERIX       MATRIX_MAP(4,0)
#define MAT_PET_KEY_5             MATRIX_MAP(4,1)

#define MAT_PET_KEY_COLON         MATRIX_MAP(4,3)
#define MAT_PET_KEY_K             MATRIX_MAP(4,4)
#define MAT_PET_KEY_H             MATRIX_MAP(4,5)
#define MAT_PET_KEY_F             MATRIX_MAP(4,6)
#define MAT_PET_KEY_S             MATRIX_MAP(4,7)

#define MAT_PET_KEY_6             MATRIX_MAP(5,0)
#define MAT_PET_KEY_4             MATRIX_MAP(5,1)

#define MAT_PET_KEY_L             MATRIX_MAP(5,3)
#define MAT_PET_KEY_J             MATRIX_MAP(5,4)
#define MAT_PET_KEY_G             MATRIX_MAP(5,5)
#define MAT_PET_KEY_D             MATRIX_MAP(5,6)
#define MAT_PET_KEY_A             MATRIX_MAP(5,7)

#define MAT_PET_KEY_SLASH         MATRIX_MAP(6,0)
#define MAT_PET_KEY_8             MATRIX_MAP(6,1)

#define MAT_PET_KEY_P             MATRIX_MAP(6,3)
#define MAT_PET_KEY_I             MATRIX_MAP(6,4)
#define MAT_PET_KEY_Y             MATRIX_MAP(6,5)
#define MAT_PET_KEY_R             MATRIX_MAP(6,6)
#define MAT_PET_KEY_W            MATRIX_MAP(6,7)

#define MAT_PET_KEY_9            MATRIX_MAP(7,0)
#define MAT_PET_KEY_7            MATRIX_MAP(7,1)
#define MAT_PET_KEY_UP_ARROW     MATRIX_MAP(7,2)
#define MAT_PET_KEY_O            MATRIX_MAP(7,3)
#define MAT_PET_KEY_U            MATRIX_MAP(7,4)
#define MAT_PET_KEY_T            MATRIX_MAP(7,5)
#define MAT_PET_KEY_E            MATRIX_MAP(7,6)
#define MAT_PET_KEY_Q            MATRIX_MAP(7,7)

#define MAT_PET_KEY_DELETE       MATRIX_MAP(8,0)
#define MAT_PET_KEY_CRSR_DOWN    MATRIX_MAP(8,1)

#define MAT_PET_KEY_RIGHT_PAREN  MATRIX_MAP(8,3)
#define MAT_PET_KEY_BACKSLASH    MATRIX_MAP(8,4)
#define MAT_PET_KEY_APOSTROPHE   MATRIX_MAP(8,5)
#define MAT_PET_KEY_DOLLAR_SIGN  MATRIX_MAP(8,6)
#define MAT_PET_KEY_DOUBLE_QUOTE MATRIX_MAP(8,7)

#define MAT_PET_KEY_CRSR_RIGHT   MATRIX_MAP(9,0)
#define MAT_PET_KEY_HOME         MATRIX_MAP(9,1)
#define MAT_PET_KEY_LEFT_ARROW   MATRIX_MAP(9,2)
#define MAT_PET_KEY_LEFT_PAREN   MATRIX_MAP(9,3)
#define MAT_PET_KEY_AMPERSAND    MATRIX_MAP(9,4)
#define MAT_PET_KEY_PERCENT      MATRIX_MAP(9,5)
#define MAT_PET_KEY_HASH         MATRIX_MAP(9,6)
#define MAT_PET_KEY_EXCLAMATION  MATRIX_MAP(9,7)

#define SCAN_C64_KEY_DELETE      SCAN_MAP(0,0)
#define SCAN_C64_KEY_RETURN      SCAN_MAP(0,1)
#define SCAN_C64_KEY_CRSR_RIGHT  SCAN_MAP(0,2)
#define SCAN_C64_KEY_F7          SCAN_MAP(0,3)
#define SCAN_C64_KEY_F1          SCAN_MAP(0,4)
#define SCAN_C64_KEY_F3          SCAN_MAP(0,5)
#define SCAN_C64_KEY_F5          SCAN_MAP(0,6)
#define SCAN_C64_KEY_CRSR_DOWN   SCAN_MAP(0,7)

#define SCAN_C64_KEY_3           SCAN_MAP(1,0)
#define SCAN_C64_KEY_W           SCAN_MAP(1,1)
#define SCAN_C64_KEY_A           SCAN_MAP(1,2)
#define SCAN_C64_KEY_4           SCAN_MAP(1,3)
#define SCAN_C64_KEY_Z           SCAN_MAP(1,4)
#define SCAN_C64_KEY_S           SCAN_MAP(1,5)
#define SCAN_C64_KEY_E           SCAN_MAP(1,6)
#define SCAN_C64_KEY_LSHIFT      SCAN_MAP(1,7)

#define SCAN_C64_KEY_5           SCAN_MAP(2,0)
#define SCAN_C64_KEY_R           SCAN_MAP(2,1)
#define SCAN_C64_KEY_D           SCAN_MAP(2,2)
#define SCAN_C64_KEY_6           SCAN_MAP(2,3)
#define SCAN_C64_KEY_C           SCAN_MAP(2,4)
#define SCAN_C64_KEY_F           SCAN_MAP(2,5)
#define SCAN_C64_KEY_T           SCAN_MAP(2,6)
#define SCAN_C64_KEY_X           SCAN_MAP(2,7)

#define SCAN_C64_KEY_7           SCAN_MAP(3,0)
#define SCAN_C64_KEY_Y           SCAN_MAP(3,1)
#define SCAN_C64_KEY_G           SCAN_MAP(3,2)
#define SCAN_C64_KEY_8           SCAN_MAP(3,3)
#define SCAN_C64_KEY_B           SCAN_MAP(3,4)
#define SCAN_C64_KEY_H           SCAN_MAP(3,5)
#define SCAN_C64_KEY_U           SCAN_MAP(3,6)
#define SCAN_C64_KEY_V           SCAN_MAP(3,7)

#define SCAN_C64_KEY_9           SCAN_MAP(4,0)
#define SCAN_C64_KEY_I           SCAN_MAP(4,1)
#define SCAN_C64_KEY_J           SCAN_MAP(4,2)
#define SCAN_C64_KEY_0           SCAN_MAP(4,3)
#define SCAN_C64_KEY_M           SCAN_MAP(4,4)
#define SCAN_C64_KEY_K           SCAN_MAP(4,5)
#define SCAN_C64_KEY_O           SCAN_MAP(4,6)
#define SCAN_C64_KEY_N           SCAN_MAP(4,7)

#define SCAN_C64_KEY_PLUS        SCAN_MAP(5,0)
#define SCAN_C64_KEY_P           SCAN_MAP(5,1)
#define SCAN_C64_KEY_L           SCAN_MAP(5,2)
#define SCAN_C64_KEY_MINUS       SCAN_MAP(5,3)
#define SCAN_C64_KEY_PERIOD      SCAN_MAP(5,4)
#define SCAN_C64_KEY_COLON       SCAN_MAP(5,5)
#define SCAN_C64_KEY_AT          SCAN_MAP(5,6)
#define SCAN_C64_KEY_COMMA       SCAN_MAP(5,7)

#define SCAN_C64_KEY_POUND       SCAN_MAP(6,0)
#define SCAN_C64_KEY_ASTERIX     SCAN_MAP(6,1)
#define SCAN_C64_KEY_SEMICOLON   SCAN_MAP(6,2)
#define SCAN_C64_KEY_HOME        SCAN_MAP(6,3)
#define SCAN_C64_KEY_RSHIFT      SCAN_MAP(6,4)
#define SCAN_C64_KEY_EQUALS      SCAN_MAP(6,5)
#define SCAN_C64_KEY_UP_ARROW    SCAN_MAP(6,6)
#define SCAN_C64_KEY_SLASH       SCAN_MAP(6,7)

#define SCAN_C64_KEY_1           SCAN_MAP(7,0)
#define SCAN_C64_KEY_LEFT_ARROW  SCAN_MAP(7,1)
#define SCAN_C64_KEY_CTRL        SCAN_MAP(7,2)
#define SCAN_C64_KEY_2           SCAN_MAP(7,3)
#define SCAN_C64_KEY_SPACE       SCAN_MAP(7,4)
#define SCAN_C64_KEY_CBM         SCAN_MAP(7,5)
#define SCAN_C64_KEY_Q           SCAN_MAP(7,6)
#define SCAN_C64_KEY_RUN_STOP    SCAN_MAP(7,7)

#endif //VKB_PET_H
