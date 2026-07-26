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

// スレーブ側: マスターから受信したsync_dataでライティングを更新するRPCハンドラ。
// transaction_register_rpc(USER_SYNC_KEY_COUNTER, ...) に渡すこと。
void lighting_tracking_rpc_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data);
