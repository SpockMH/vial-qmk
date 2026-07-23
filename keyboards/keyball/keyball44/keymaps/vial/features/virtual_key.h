#pragma once
#include <stdint.h>
#include <stdbool.h>

// keymap.c側で定義する、ユーザー定義キーコード(0x7E00以降)のディスパッチ関数。
// switch文で個々のキーコードに対応する関数を直接呼び出す。
// AZ1UBALLのdpad/gesture等、process_record_userを経由しない経路から
// virtual_key.c経由で呼ばれる。
bool process_user_virtual_keycode(uint16_t keycode);

// AZ1UBALLのdpad/gesture機能から、仮想マトリクス位置(row,col)に割り当てられた
// キーを1回分「実行」する。
//   - 通常のキーコード(KC_A等)          → tap_code16()でHID出力する
//   - ユーザー定義キーコード(0x7E00以降) → process_user_virtual_keycode()に委譲し、
//                                          対応する関数を直接呼び出す
void fire_virtual_key(uint8_t row, uint8_t col);