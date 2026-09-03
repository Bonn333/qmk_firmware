// Copyright 2026 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
//
// This board does not scan its matrix the way QMK does by default.
//
// Recovered from the stock firmware (scan routine at 0x2840..0x2AC6):
//   1. all 21 column lines are driven LOW
//   2. the six row lines are briefly driven LOW as outputs, to discharge them
//   3. the row lines become plain inputs with NO pull-up (SN32 CFG = 2)
//   4. exactly one column is driven HIGH
//   5. a row reading HIGH means the key at that intersection is pressed
//
// QMK's default matrix does the opposite on every count, and the switch diodes
// are oriented so that pulling a column low reverse-biases them - a raw HID
// sweep of the hardware confirmed the default scan detects nothing at all.
//
// matrix_read_rows_on_col() and matrix_init_pins() are weak in quantum/matrix.c
// so they are replaced here. This runs inside the RGB PWM interrupt, so it goes
// straight at the registers instead of through PAL: the row pins sit on exactly
// two ports, which turns the discharge into six register writes rather than
// twelve PAL calls, and the read into two loads rather than six.

#include "quantum.h"
#include "matrix.h"
#include "timer.h"

#define SCAN_GRACE_MS 1500

// Rows: C15 on port 2, D7..D11 on port 3. Keep in sync with MATRIX_ROW_PINS.
#define ROWMASK_P2 (1u << 15)
#define ROWMASK_P3 (0x1Fu << 7)

// Columns: A0..A14 on port 0, B0..B5 on port 1. Driving all 21 one call at a
// time cost 42 PAL writes per scan and measurably ate into the LED duty cycle -
// the whole board went dim. Two register writes per direction instead. They are
// left high, the non-conducting level, so nothing lights between scans.
#define COLMASK_P0 (0x7FFFu)
#define COLMASK_P1 (0x3Fu)

static const pin_t gk_col_pins[MATRIX_COLS] = MATRIX_COL_PINS;

static inline void rows_discharge_then_float(void) {
    SN_GPIO2->MODE |= ROWMASK_P2;  // drive low briefly to bleed off the line
    SN_GPIO2->DATA &= ~ROWMASK_P2;
    SN_GPIO3->MODE |= ROWMASK_P3;
    SN_GPIO3->DATA &= ~ROWMASK_P3;
    SN_GPIO2->MODE &= ~ROWMASK_P2; // back to input, no pull-up (CFG stays 2)
    SN_GPIO3->MODE &= ~ROWMASK_P3;
}

static inline uint8_t rows_read(void) {
    uint32_t p2 = SN_GPIO2->DATA;
    uint32_t p3 = SN_GPIO3->DATA;
    // bit order must match MATRIX_ROW_PINS: C15, D11, D10, D9, D8, D7
    return (uint8_t)(((p2 >> 15) & 1) << 0 | ((p3 >> 11) & 1) << 1 | ((p3 >> 10) & 1) << 2 |
                     ((p3 >> 9) & 1) << 3 | ((p3 >> 8) & 1) << 4 | ((p3 >> 7) & 1) << 5);
}

void matrix_init_pins(void) {
    for (uint8_t c = 0; c < MATRIX_COLS; c++) {
        gpio_set_pin_output_push_pull(gk_col_pins[c]);
        gpio_write_pin_low(gk_col_pins[c]);
    }
    // Set the row CFG to "input, no pull-up" once, via PAL, outside any ISR.
    const pin_t rows[MATRIX_ROWS] = MATRIX_ROW_PINS;
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        gpio_set_pin_input(rows[r]);
    }
}

void matrix_read_rows_on_col(matrix_row_t current_matrix[], uint8_t current_col, matrix_row_t row_shifter) {
    /* When SN32F2XX_PWM_DIRECTION differs from DIODE_DIRECTION the driver calls
       this in a loop over the columns but passes a row counter in current_col
       rather than the column index - see shared_matrix_scan_keys(). The real
       column is carried by row_shifter, which is always (1 << col). */
    if (row_shifter) {
        current_col = (uint8_t)__builtin_ctz(row_shifter);
    }
    if (current_col >= MATRIX_COLS) {
        return;
    }

    /* In this configuration the driver sweeps every column inside a single PWM
       interrupt, so a full matrix read happens on each pass. That is roughly
       twenty times the work the matching-direction path does, and it eats enough
       of the interrupt budget to visibly dim the whole board. Debounce is 8 ms,
       so reading every fourth sweep is still far quicker than that. The matrix
       keeps its previous bits on the skipped passes. */
    static uint8_t sweep_div = 0;
    if (row_shifter == MATRIX_ROW_SHIFTER) {
        sweep_div = (uint8_t)((sweep_div + 1) & 3);
    }
    if (sweep_div != 0) {
        return;
    }

    if (timer_read32() < SCAN_GRACE_MS) {
        return; // stay out of the way until USB is definitely up
    }

    // The RGB driver parks every column HIGH before calling this, because HIGH is
    // the LED-off level on this board. The key scan needs the opposite: every
    // unselected column LOW, and the selected one HIGH. So drive them all down
    // here, take the reading, and hand them back parked HIGH.
    SN_GPIO0->DATA &= ~COLMASK_P0;
    SN_GPIO1->DATA &= ~COLMASK_P1;
    rows_discharge_then_float();

    gpio_write_pin_high(gk_col_pins[current_col]);
    for (volatile uint8_t d = 0; d < 12; d++) { // short settle, no wait_us in an ISR
        __asm__ volatile("nop");
    }

    uint8_t bits = rows_read();
    SN_GPIO0->DATA |= COLMASK_P0; // back to the non-conducting level
    SN_GPIO1->DATA |= COLMASK_P1;

    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        if (bits & (1u << r)) {
            current_matrix[r] |= row_shifter;
        } else {
            current_matrix[r] &= ~row_shifter;
        }
    }
}
