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

#pragma once
#include "quantum.h"

// キー座標系(row/col)の定義:
//   row 0-3 = 左手側, row 4-6(スクロールレイヤー時は4-6でラップ) = 右手側
//   このモジュール内部では常に「右手側基準(4-6)」の座標で扱う。
//   左手側の座標が来た場合は下記の変換式で右手側に正規化する:
//     row' = row + 4
//     col' = 5 - col
#define LIGHTING_ROW_OFFSET 4
#define LIGHTING_COL_MAX    5 // col は 0-5

// ライティング座標追跡サブシステムの初期化(keyboard_post_init_userから呼ぶ)
void lighting_tracking_init(void);

// マウス移動量からライティング座標(row/col)を更新し、必要なら座標が
// 変化した分だけスレーブへ同期する。
// layerscr: スクロールレイヤー中かどうか(座標のラップ範囲が変わる)
void lighting_tracking_update(report_mouse_t *mouse_report, bool layerscr);

// 現在保持しているキー座標を直接指定する(押下されたキーの位置を反映する用)
void lighting_tracking_set_position(uint8_t row, uint8_t col);

// 現在の座標でライティングを即時発火し、スレーブへ同期する。
// (キー押下やクリック、レイヤー切替など、マウス移動以外のイベント用)
void lighting_tracking_trigger(bool scr, bool click);
void lighting_tracking_refresh(bool scr);

// スレーブ側: マスターから受信したsync_dataでライティングを更新するRPCハンドラ。
// transaction_register_rpc(USER_SYNC_KEY_COUNTER, ...) に渡すこと。
void lighting_tracking_rpc_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data);
