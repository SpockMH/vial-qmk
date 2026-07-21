#pragma once
#include <stdint.h>
#include <stdbool.h>
#include "quantum.h"

// 十字キー4方向用の仮想マトリクス位置(info.json上の未使用スロット)。
// Vialのキーマップ編集画面から、この4箇所に自由にキーを割り当てられる。
#define AZ_DPAD_UP_ROW    3
#define AZ_DPAD_UP_COL    0
#define AZ_DPAD_DOWN_ROW  7
#define AZ_DPAD_DOWN_COL  0
#define AZ_DPAD_LEFT_ROW  7
#define AZ_DPAD_LEFT_COL  2
#define AZ_DPAD_RIGHT_ROW 7
#define AZ_DPAD_RIGHT_COL 3

// ============================================================================
// 追加分: az1uball_dpad.c が参照しているが、元ファイルに定義がなかった型
// ============================================================================

// 確定した方向(スレーブ→マスターへRPCで渡す値)
typedef enum {
    AZ_DPAD_DIR_UP,
    AZ_DPAD_DIR_DOWN,
    AZ_DPAD_DIR_LEFT,
    AZ_DPAD_DIR_RIGHT,
} az_dpad_direction_t;

// RPCでスレーブ→マスターへ渡す同期用データ。
// マスター側の az1uball_dpad_master_task() が transaction_rpc_exec() で
// この構造体をそのまま受け取るため、両側で同一のレイアウトである必要がある。
typedef struct {
    uint8_t direction;     // az_dpad_direction_t の値
    bool    has_direction; // このポーリングで方向が確定したか
    bool    click;         // 現在のクリック(ボタン)状態
} az_dpad_sync_t;

// クリックはVialでは割り当てず、プログラム側で直接送信する。
// keymap.cの独自キーコードenumで実際の値を割り当てること。
extern uint16_t AZ_CLICK_KEYCODE;

// スレーブ側: matrix_scan_user()の中で is_keyboard_left() の時に呼ぶこと。
// AZ1UBALLの生データを読み、状態機械(IDLE→ACCUMULATING→WAIT_STOP)を回し、
// 方向が確定したらRPCでマスターへイベントを送る。ここではキー出力は行わない
// (マスター側でのみ実際のtap_code16/register_codeが実行される)。
void az1uball_dpad_task(void);

// マスター側: housekeeping_task_user() などから定期的に呼ぶこと。
// スレーブをRPCでポーリングし、確定した方向・クリック状態に応じて
// マスター自身のlayer_state/keymapsを使って実際にキーを出力する。
void az1uball_dpad_master_task(void);

// 両側の keyboard_post_init_user() から呼ぶこと(RPCハンドラの登録)。
// 実際にハンドラが呼ばれる=キーが送信されるのはマスター側だけ。
void az1uball_dpad_register_rpc(void);

// ----------------------------------------------------------------------------
// 注意: このヘッダ以外にもう1箇所、以下の追加が必要です。
//
//   transactions.h (もしくはユーザー定義の user_sync_id_t enum がある場所) に、
//   既存の USER_SYNC_KEY_COUNTER などと同じ並びで
//
//       USER_SYNC_AZ_DPAD,
//
//   を追加してください。az1uball_dpad.c 側の
//   transaction_register_rpc(USER_SYNC_AZ_DPAD, ...) /
//   transaction_rpc_exec(USER_SYNC_AZ_DPAD, ...) が参照しています。
//   このファイルの中身が見えていないため、ここでは追加できませんでした。
// ----------------------------------------------------------------------------