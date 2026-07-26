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
