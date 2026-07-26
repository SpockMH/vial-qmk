/*
 * Copyright 2017 Alex Ong <the.onga@gmail.com>
 * Copyright 2020 Andrei Purdea <andrei@purdea.ro>
 * Copyright 2021 Simon Arlott
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
Basic symmetric per-key algorithm. Uses an 8-bit counter per key.
When no state changes have occured for DEBOUNCE milliseconds, we push the state.
*/

// modified for BMP by sekigon-gonnoc

#include "matrix.h"
#include "timer.h"
#include "quantum.h"

#define ROW_SHIFTER ((matrix_row_t)1)
#define EVENT_CNT_MAX 16

typedef struct {
    bool    pressed : 1;
    uint8_t time : 7;
} debounce_counter_t;

static debounce_counter_t  debounce_counters[MATRIX_ROWS * MATRIX_COLS];
static fast_timer_t        last_time;
static bool                counters_need_update;
static bool                matrix_need_update;
static bool                cooked_changed;
static uint8_t             event_count = 0;

#define DEBOUNCE_ELAPSED 0

static void update_debounce_counters_and_transfer_if_expired(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, uint8_t num_cols, uint8_t elapsed_time, bmp_api_key_event_t *events);
static void transfer_matrix_values(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, uint8_t num_cols, uint8_t debounce_value, bmp_api_key_event_t *events);

// we use num_rows rather than MATRIX_ROWS to support split keyboards
void bmp_debounce_init(void) {
    int i = 0;
    for (uint8_t r = 0; r < MATRIX_ROWS; r++) {
        for (uint8_t c = 0; c < MATRIX_COLS; c++) {
            debounce_counters[i++].time = DEBOUNCE_ELAPSED;
        }
    }
}

int bmp_debounce(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows,
             uint8_t num_cols, uint8_t debounce_value, bool changed,
             bmp_api_key_event_t *events) {
    bool updated_last = false;
    cooked_changed    = false;
    event_count = 0;

    debounce_value = debounce_value > 127 ? 127 : debounce_value;

    if (counters_need_update) {
        fast_timer_t now = timer_read_fast();
        fast_timer_t elapsed_time = TIMER_DIFF_FAST(now, last_time);

        last_time = now;
        updated_last = true;
        if (elapsed_time > UINT8_MAX) {
            elapsed_time = UINT8_MAX;
        }

        if (elapsed_time > 0) {
            update_debounce_counters_and_transfer_if_expired(raw, cooked, num_rows, num_cols, elapsed_time, events);
        }
    }

    if (changed || matrix_need_update) {
        if (!updated_last) {
            last_time = timer_read_fast();
        }

        transfer_matrix_values(raw, cooked, num_rows, num_cols, debounce_value, events);
    }

    return event_count;
}

static void update_debounce_counters_and_transfer_if_expired(matrix_row_t raw[], matrix_row_t cooked[], uint8_t num_rows, uint8_t num_cols, uint8_t elapsed_time, bmp_api_key_event_t *events) {
    debounce_counter_t *debounce_pointer = debounce_counters;

    counters_need_update = false;
    matrix_need_update   = false;

    for (uint8_t row = 0; row < num_rows; row++) {
        for (uint8_t col = 0; col < num_cols; col++) {
            matrix_row_t col_mask = (ROW_SHIFTER << col);

            if (debounce_pointer->time != DEBOUNCE_ELAPSED) {
                if (debounce_pointer->time <= elapsed_time) {
                    debounce_pointer->time = DEBOUNCE_ELAPSED;

                    if (debounce_pointer->pressed) {
                        // key-down: eager
                        matrix_need_update = true;
                    } else {
                        // key-up: defer
                        matrix_row_t cooked_next = (cooked[row] & ~col_mask) | (raw[row] & col_mask);
                        cooked_changed |= cooked_next ^ cooked[row];
                        cooked[row] = cooked_next;

                        events[event_count].row = row;
                        events[event_count].col = col;
                        events[event_count].state = (cooked_next & col_mask) ? 1 : 0;
                        if (event_count < EVENT_CNT_MAX - 1) event_count++;
                    }
                } else {
                    debounce_pointer->time -= elapsed_time;
                    counters_need_update = true;
                }
            }
            debounce_pointer++;
        }
    }
}

// upload from raw_matrix to final matrix;
static void transfer_matrix_values(matrix_row_t raw[], matrix_row_t cooked[],
                                   uint8_t num_rows, uint8_t num_cols,
                                   uint8_t debounce_value, bmp_api_key_event_t *events) {
    debounce_counter_t *debounce_pointer = debounce_counters;

    matrix_need_update  = false;

    for (uint8_t row = 0; row < num_rows; row++) {
        matrix_row_t delta = raw[row] ^ cooked[row];
        for (uint8_t col = 0; col < num_cols; col++) {
            matrix_row_t col_mask = (ROW_SHIFTER << col);

            if (delta & col_mask) {
                if (debounce_pointer->time == DEBOUNCE_ELAPSED) {
                    debounce_pointer->pressed = (raw[row] & col_mask);
                    debounce_pointer->time = debounce_value;
                    counters_need_update = true;

                    if (debounce_pointer->pressed) {
                        // key-down: eager
                        cooked[row] ^= col_mask;
                        cooked_changed = true;

                        events[event_count].row = row;
                        events[event_count].col = col;
                        events[event_count].state = (cooked[row] & col_mask) ? 1 : 0;
                        if (event_count < EVENT_CNT_MAX - 1) event_count++;
                    }
                }
            } else if (debounce_pointer->time != DEBOUNCE_ELAPSED) {
                if (!debounce_pointer->pressed) {
                    // key-up: defer
                    debounce_pointer->time = DEBOUNCE_ELAPSED;
                }
            }
            debounce_pointer++;
        }
    }
}

// bool debounce_active(void) { return true; }
