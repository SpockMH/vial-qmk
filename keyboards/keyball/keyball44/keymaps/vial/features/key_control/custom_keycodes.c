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

#include "custom_keycodes.h"
#include "jis2us.h"
#include "select_extend.h"
#include "features/pointing/mouse_mode.h"
#include "features/lighting/rgblight_user.h"
#include "features/config/eeconfig_user.h"

// select_extend.h / mouse_speed_smoothing.h 側の「weakなextern変数」に
// 実際のキーコード値を紐付ける(select_word_lr.h と同じパターン)。
uint16_t SELECT_WORD_LEFT_KEYCODE  = SELWORD_LEFT;
uint16_t SELECT_WORD_RIGHT_KEYCODE = SELWORD_RIGHT;
uint16_t SELECT_LINE_UP_KEYCODE    = SELLINE_UP;
uint16_t SELECT_LINE_DOWN_KEYCODE  = SELLINE_DOWN;
uint16_t SPD_THL_UP_KEYCODE = SPD_THL_UP;
uint16_t SPD_THL_DN_KEYCODE = SPD_THL_DN;
uint16_t SPD_THU_UP_KEYCODE = SPD_THU_UP;
uint16_t SPD_THU_DN_KEYCODE = SPD_THU_DN;
uint16_t SPD_SCL_UP_KEYCODE = SPD_SCL_UP;
uint16_t SPD_SCL_DN_KEYCODE = SPD_SCL_DN;

bool process_user_keycode(uint16_t keycode, keyrecord_t *record) {
  if (!record->event.pressed) return true;
  switch (keycode) {
    case JPUS_TOG:
        jis2us_toggle();
        return false;

    case MMD_TOG:
        mouse_mode_toggle();
        return false;

    case MMT_DEC:
        mouse_mode_change_term(false);
        return false;

    case MMT_INC:
        mouse_mode_change_term(true);
        return false;

    case LIGHT_TOG:
        rgblight_toggle();
        return false;

    case LIGHT_VAI:
        rgblight_increase_val();
        return false;

    case LIGHT_VAD:
        rgblight_decrease_val();
        return false;

    case HUE_UP:
        set_hue(get_hue() + 3);
        return false;

    case KBC_SAVE:
        save_user_config();
        return false;

    case SELWORD_LEFT:
        select_extend_word_left();
        return false;

    case SELWORD_RIGHT:
        select_extend_word_right();
        return false;

    case SELLINE_UP:
        select_extend_line_up();
        return false;

    case SELLINE_DOWN:
        select_extend_line_down();
        return false;
  }
  return true;
}

bool process_user_virtual_keycode(uint16_t keycode) {
  switch (keycode) {
    case SELWORD_LEFT:
        select_extend_word_left();
        return false;

    case SELWORD_RIGHT:
        select_extend_word_right();
        return false;
      
    case SELLINE_UP:
        select_extend_line_up();
        return false;

    case SELLINE_DOWN:
        select_extend_line_down();
        return false;

    case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX: {
      // キーコードからレイヤー番号 (0〜31) を抽出
      uint8_t layer = QK_TOGGLE_LAYER_GET_LAYER(keycode);

      if (layer_state_is(layer)) {
        layer_off(layer);
      } else {
        layer_on(layer);
      }
      return false;
    }
    default:
      return true; // 未対応のキーコードは無視
  }
}
