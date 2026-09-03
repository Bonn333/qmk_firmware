// Copyright 2026 QMK
// SPDX-License-Identifier: GPL-2.0-or-later

#pragma once

/* Only the SN32F2xx RGB driver settings live here; everything with a data driven
   equivalent is in keyboard.json. */

/* PWM on the columns, which are the current sinks, and the rows as plain GPIO
   anodes - the same arrangement the stock firmware uses. DIODE_DIRECTION stays
   ROW2COL, so this deliberately differs from it: that is what makes the driver
   compile the branch whose shared_matrix_rgb_disable_output() switches the
   column PWM channels off and drives every row pin to the off level before each
   key scan. With PWM on the rows that blanking is compiled out, a column only
   conducts during a narrow window, the higher forward voltage channels never get
   going, and every colour mix collapses towards red. */
#define SN32F2XX_PWM_DIRECTION COL2ROW

/* Rows are the anodes; the stock firmware activates one with GPIO_BSET. */
#define SN32F2XX_RGB_OUTPUT_ACTIVE_LEVEL SN32F2XX_RGB_OUTPUT_ACTIVE_HIGH

/* Columns are the cathode side, so the PWM pulls them low to light an LED. */
#define SN32F2XX_PWM_OUTPUT_ACTIVE_LEVEL SN32F2XX_PWM_OUTPUT_ACTIVE_LOW

/* Three anode pins per matrix row, in { R, B, G } order - the order the driver
   assigns a triple. Confirmed against hardware rather than assumed: the stock
   firmware scans its seventh LED row as A15, C14, C13 and those light the side
   strips red, green and blue respectively, so the stock channel order is R, G, B
   and row 0 (stock C0, C3, C1) becomes { C0, C1, C3 } here. */
#define SN32F2XX_RGB_MATRIX_ROW_PINS { C0, C1, C3, C4, C5, C6, C7, C8, C9, C10, C11, C12, B6, B7, B8, B9, B10, B11 }

/* Six of the animations selected in keyboard.json need a switch of their own on
   top of being listed there: the reactive and splash effects want key event
   tracking, and typing_heatmap and digital_rain need the effect framebuffer.
   Without these two the entries are silently compiled out. */
#define RGB_MATRIX_KEYPRESSES
#define RGB_MATRIX_FRAMEBUFFER_EFFECTS
