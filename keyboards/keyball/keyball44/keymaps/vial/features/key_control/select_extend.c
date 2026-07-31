/**
 * Copyright (c) 2026 SpockMH
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACTText, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "select_extend.h"

__attribute__((weak)) uint16_t SELECT_WORD_LEFT_KEYCODE   = KC_NO;
__attribute__((weak)) uint16_t SELECT_WORD_RIGHT_KEYCODE  = KC_NO;
__attribute__((weak)) uint16_t SELECT_LINE_UP_KEYCODE     = KC_NO;
__attribute__((weak)) uint16_t SELECT_LINE_DOWN_KEYCODE   = KC_NO;


static bool selection_active = false;

static void tap_ctrl(uint16_t kc) {
    register_code(KC_LCTL);
    tap_code(kc);
    unregister_code(KC_LCTL);
}

static void tap_shift(uint16_t kc) {
    register_code(KC_LSFT);
    tap_code(kc);
    unregister_code(KC_LSFT);
}

static void tap_ctrl_shift(uint16_t kc) {
    register_code(KC_LCTL);
    register_code(KC_LSFT);
    tap_code(kc);
    unregister_code(KC_LSFT);
    unregister_code(KC_LCTL);
}

void select_extend_word_left(void) {
    if (!selection_active) {
        tap_ctrl(KC_RIGHT);
        tap_ctrl_shift(KC_LEFT);
        selection_active = true;
    } else {
        tap_ctrl_shift(KC_LEFT);
    }
}

void select_extend_word_right(void) {
    if (!selection_active) {
        tap_ctrl(KC_LEFT);
        tap_ctrl_shift(KC_RIGHT);
        selection_active = true;
    } else {
        tap_ctrl_shift(KC_RIGHT);
    }
}

void select_extend_line_up(void) {
    if (!selection_active) {
        tap_code(KC_END);
        tap_shift(KC_HOME);
        selection_active = true;
    } else {
        tap_shift(KC_UP);
        tap_shift(KC_HOME);
    }
}

void select_extend_line_down(void) {
    if (!selection_active) {
        tap_code(KC_HOME);
        tap_shift(KC_END);
        selection_active = true;
    } else {
        tap_shift(KC_DOWN);
        tap_shift(KC_END);
    }
}

void select_extend_reset(void) {
    selection_active = false;
}

bool process_select_extend(uint16_t keycode, keyrecord_t* record) {
    bool is_word_left  = (keycode == SELECT_WORD_LEFT_KEYCODE);
    bool is_word_right = (keycode == SELECT_WORD_RIGHT_KEYCODE);
    bool is_line_up    = (keycode == SELECT_LINE_UP_KEYCODE);
    bool is_line_down  = (keycode == SELECT_LINE_DOWN_KEYCODE);


    if (is_word_left || is_word_right || is_line_up || is_line_down) {
        if (!record->event.pressed) {
            return false;
        }
        if (is_word_left)       select_extend_word_left();
        else if (is_word_right) select_extend_word_right();
        else if (is_line_up)    select_extend_line_up();
        else                    select_extend_line_down();
        return false;
    }

    // 4キー以外が押されたら選択モード解除
    if (record->event.pressed && selection_active) {
        select_extend_reset();
    }
    return true;
}