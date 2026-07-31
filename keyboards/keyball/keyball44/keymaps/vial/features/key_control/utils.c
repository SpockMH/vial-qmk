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

#include "utils.h"
#include "custom_keycodes.h"
#include "lib/keyball/keyball.h"

bool process_cpi_x3(uint16_t keycode, keyrecord_t *record) {
  if (keycode != CPI_x3) return true;
  if (record->event.pressed) {
    keyball_set_cpi(keyball_get_cpi()*3);
  } else {
    keyball_set_cpi(keyball_get_cpi()/3);
  }
  return false;
}

bool process_kana_ime(uint16_t keycode, keyrecord_t *record) {
  static bool kana = false;

  if (keycode == KC_INTERNATIONAL_5) {
    kana = false;
  } else if (keycode == KC_INTERNATIONAL_4) {
    kana = true;
  }

  // 変換候補ウィンドウ操作のキーコード範囲。離した時にkana状態ならIME ONへ戻す。
  if (keycode >= 0x7700 && keycode <= 0x7703) {
    if (!record->event.pressed && kana) {
      tap_code16(KC_INTERNATIONAL_4);
    }
  }

  if (keycode == KC_LEFT_BRACKET) {
    if (record->event.pressed) {
      tap_code16(KC_INTERNATIONAL_5);
      return true;
    }
  }

  return true;
}
