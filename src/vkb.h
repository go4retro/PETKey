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
 *  poll.h: Definitions for matrix keyboard to CBM computer scanning routine
 */

#ifndef VKB_H
#define VKB_H

#define SCAN_MAP(r,c)            ( \
                                  ( \
                                   r == 0 ? SCAN_ROW_0 : \
                                   r == 1 ? SCAN_ROW_1 : \
                                   r == 2 ? SCAN_ROW_2 : \
                                   r == 3 ? SCAN_ROW_3 : \
                                   r == 4 ? SCAN_ROW_4 : \
                                   r == 5 ? SCAN_ROW_5 : \
                                   r == 6 ? SCAN_ROW_6 : SCAN_ROW_7 \
                                  ) \
                                 | \
                                  ( \
                                   c == 0 ? SCAN_COL_0 : \
                                   c == 1 ? SCAN_COL_1 : \
                                   c == 2 ? SCAN_COL_2 : \
                                   c == 3 ? SCAN_COL_3 : \
                                   c == 4 ? SCAN_COL_4 : \
                                   c == 5 ? SCAN_COL_5 : \
                                   c == 6 ? SCAN_COL_6 : SCAN_COL_7 \
                                  ) << 3 \
                                 )

#define MATRIX_MAP(x,y)         ( \
                                 ( \
                                  ( \
                                   x == 0 ? XPT_ROW_0 : \
                                   x == 1 ? XPT_ROW_1 : \
                                   x == 2 ? XPT_ROW_2 : \
                                   x == 3 ? XPT_ROW_3 : \
                                   x == 4 ? XPT_ROW_4 : \
                                   x == 5 ? XPT_ROW_5 : \
                                   x == 6 ? XPT_ROW_6 : \
                                   x == 7 ? XPT_ROW_7 : \
                                   x == 8 ? XPT_ROW_8 : XPT_ROW_9 \
                                  ) \
                                 ) \
                                | \
                                 ( \
                                  ( \
                                   y == 0 ? XPT_COL_0 :   \
                                   y == 1 ? XPT_COL_1 :   \
                                   y == 2 ? XPT_COL_2 :   \
                                   y == 3 ? XPT_COL_3 :   \
                                   y == 4 ? XPT_COL_4 :   \
                                   y == 5 ? XPT_COL_5 :   \
                                   y == 6 ? XPT_COL_6 : XPT_COL_7 \
                                  ) \
                                 ) << 4\
                                )
void vkb_init(void);
void vkb_irq(void);
void vkb_scan(void);

#endif
