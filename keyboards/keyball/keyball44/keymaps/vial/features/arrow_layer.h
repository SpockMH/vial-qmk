#pragma once
#include <stdint.h>
#include "quantum.h"

/**
 * process_arrow_layer
 *
 * トラックボールの動き(x, y)をテンション式の方向キー入力に変換する。
 * 一定量動くたびに up/down/left/right のいずれかのキーコードを1回 tap する。
 *
 * 呼び出し側(pointing_device_task_user等)の責務:
 *   - どのレイヤー/モードでこの関数を呼ぶか判定する
 *   - 呼んだ後に mouse_report.x/y = 0 にするかどうかを決める
 *   - 渡すキーコードを決める (KC_NO を渡せばそのキーは常に無効)
 *
 * この関数の責務:
 *   - x/y の動き量をテンションとして蓄積する
 *   - テンションが閾値を超えたら対応するキーコードを1回 tap する
 *   - 縦方向に動いた直後は一定時間横方向の発火を抑制する(誤操作防止)
 *
 * @param x      X軸の動き量 (右が正)
 * @param y      Y軸の動き量 (下が正)
 * @param up     上方向で発火するキーコード (KC_NO で無効)
 * @param down   下方向で発火するキーコード (KC_NO で無効)
 * @param left   左方向で発火するキーコード (KC_NO で無効)
 * @param right  右方向で発火するキーコード (KC_NO で無効)
 */
void process_arrow_layer(int16_t x, int16_t y, uint16_t up, uint16_t down, uint16_t left, uint16_t right);

/**
 * process_arrow_layer の内部テンション/タイマー状態をリセットする。
 * レイヤー切替時や az1uball モード切替時に呼ぶこと。
 */
void process_arrow_layer_reset(void);