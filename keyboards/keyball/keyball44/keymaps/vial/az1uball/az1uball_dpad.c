/*
 * az1uball_dpad.c
 *
 * AZ1UBALLをマウスポインタではなく、十字キー+クリックとして扱う。
 *
 * 重要な設計上の制約:
 *   AZ1UBALLはスレーブ(左手)側のハードウェアだが、キー出力(USBレポート送信)は
 *   マスター側でしか正しく機能しない。また layer_state や Vialの動的キーマップも
 *   マスター側でしか正確とは限らない。そのため:
 *
 *     - スレーブ: az1uball_dpad_task() で状態機械を回し、「方向が確定したか」
 *       「現在のクリック状態」をローカルに保持するだけ(キー出力は一切しない)
 *     - マスター: az1uball_dpad_master_task() が定期的にスレーブへRPCで問い合わせ、
 *       確定していればマスター自身の正しいlayer_state/keymapsを使って実際に
 *       キーを出力する
 *
 *   RPCは常にマスターが起点(スレーブから自発的には送れない)という制約があるため、
 *   「マスターがポーリングする」形になっている。
 *
 * 状態機械の仕様(1回の操作につき、確実に1回だけ方向を確定する):
 *   IDLE(待機)
 *     → 動作を検知した瞬間にタイマー開始、ACCUMULATING へ
 *   ACCUMULATING(30ms計測中)
 *     → 動作量を蓄積し続ける
 *     → 動作検知から30ms経過したら、蓄積値のうち最大の方向を確定する
 *       (閾値5未満なら確定しない)。WAIT_STOP へ
 *   WAIT_STOP(停止待ち)
 *     → 動作が止まった瞬間にタイマー開始
 *     → 停止から10ms経過したら IDLE に戻り、次の操作を受け付け可能にする
 *     → 途中で動作が再開したら停止タイマーはリセット(WAIT_STOPのまま)
 *
 * NOTE: az_dpad_direction_t / az_dpad_sync_t の型定義は az1uball_dpad.h に
 *       移動しました(元ファイルには定義が無く、コンパイルできませんでした)。
 */

#include "az1uball_dpad.h"
#include "az1uball.h"
#include "transactions.h"

#define AZ_DPAD_WINDOW_MS       30 // 動作検知からこの時間、動作量を蓄積してから判定する
#define AZ_DPAD_IDLE_MS         10 // 停止からこの時間経過したら、次の操作を受け付ける
#define AZ_DPAD_THRESHOLD       5  // 蓄積値がこの値未満なら、方向を確定しない
#define AZ_DPAD_POLL_INTERVAL_MS 10 // マスターがスレーブをポーリングする間隔

__attribute__((weak)) uint16_t AZ_CLICK_KEYCODE = KC_NO;

// ============================================================================
// スレーブ側: 状態機械(キー出力は一切行わない)
// ============================================================================

typedef enum {
    AZ_DPAD_IDLE,
    AZ_DPAD_ACCUMULATING,
    AZ_DPAD_WAIT_STOP,
} az_dpad_state_t;

static az_dpad_state_t state        = AZ_DPAD_IDLE;
static uint32_t        window_start = 0;
static uint32_t        idle_start   = 0;
static int16_t         accum_h      = 0;
static int16_t         accum_v      = 0;

// マスターに渡す用の「確定した方向・現在のクリック状態」。
// この構造体自体はスレーブ側のローカル変数で、RPCハンドラ経由でのみ外部に公開される。
static az_dpad_sync_t pending_sync = {0, false, false};

// accum_h/accum_vから、上下左右のうち最大の値と対応する方向を求める。
// 閾値以上ならpending_syncに確定方向をセットする。
static void evaluate_and_set_pending(void) {
    int16_t right = accum_h > 0 ? accum_h : 0;
    int16_t left  = accum_h < 0 ? (int16_t)(-accum_h) : 0;
    int16_t down  = accum_v > 0 ? accum_v : 0;
    int16_t up    = accum_v < 0 ? (int16_t)(-accum_v) : 0;

    int16_t             max_val = right;
    az_dpad_direction_t dir     = AZ_DPAD_DIR_RIGHT;

    if (left > max_val) {
        max_val = left;
        dir     = AZ_DPAD_DIR_LEFT;
    }
    if (down > max_val) {
        max_val = down;
        dir     = AZ_DPAD_DIR_DOWN;
    }
    if (up > max_val) {
        max_val = up;
        dir     = AZ_DPAD_DIR_UP;
    }

    if (max_val >= AZ_DPAD_THRESHOLD) {
        pending_sync.direction     = (uint8_t)dir;
        pending_sync.has_direction = true;
    }
}

void az1uball_dpad_task(void) {
    int16_t x = 0, y = 0;
    bool    btn = false;
    az1uball_get_motion(&x, &y, &btn);

    // クリック状態は常に最新をpending_syncに反映しておくだけ(送信・出力はマスター側)
    pending_sync.click = btn;

    bool moving = (x != 0 || y != 0);

    switch (state) {
        case AZ_DPAD_IDLE:
            if (moving) {
                window_start = timer_read32();
                accum_h      = x;
                accum_v      = y;
                state        = AZ_DPAD_ACCUMULATING;
            }
            break;

        case AZ_DPAD_ACCUMULATING:
            accum_h += x;
            accum_v += y;
            if (timer_elapsed32(window_start) >= AZ_DPAD_WINDOW_MS) {
                evaluate_and_set_pending();
                state      = AZ_DPAD_WAIT_STOP;
                idle_start = 0;
            }
            break;

        case AZ_DPAD_WAIT_STOP:
            if (moving) {
                idle_start = 0;
            } else if (idle_start == 0) {
                idle_start = timer_read32();
            } else if (timer_elapsed32(idle_start) >= AZ_DPAD_IDLE_MS) {
                state      = AZ_DPAD_IDLE;
                idle_start = 0;
            }
            break;
    }
}

// マスターからのポーリングに応答する(スレーブ側で実行される)。
// 呼ばれた時点のpending_syncを返し、has_directionはここでクリアする
// (1回のポーリングで1回だけ消費させ、二重発火を防ぐ)。
static void rpc_az_dpad_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    *(az_dpad_sync_t *)out_data = pending_sync;
    pending_sync.has_direction  = false;
}

// ============================================================================
// マスター側: ポーリング+実際のキー出力
// ============================================================================

// 現在アクティブなレイヤーで、指定した仮想位置(row,col)に割り当てられている
// キーコードを1回タップする。マスター自身の正しいlayer_state/keymapsを使う。
static void fire_virtual_key(uint8_t row, uint8_t col) {
    keypos_t pos      = {.row = row, .col = col};
    uint8_t  layer     = get_highest_layer(layer_state);
    uint16_t assigned  = keymap_key_to_keycode(layer, pos);

    if (assigned == KC_NO || assigned == KC_TRNS) {
        return;
    }

    tap_code16(assigned);
}

static void direction_to_rowcol(az_dpad_direction_t dir, uint8_t *row, uint8_t *col) {
    switch (dir) {
        case AZ_DPAD_DIR_UP:
            *row = AZ_DPAD_UP_ROW;
            *col = AZ_DPAD_UP_COL;
            break;
        case AZ_DPAD_DIR_DOWN:
            *row = AZ_DPAD_DOWN_ROW;
            *col = AZ_DPAD_DOWN_COL;
            break;
        case AZ_DPAD_DIR_LEFT:
            *row = AZ_DPAD_LEFT_ROW;
            *col = AZ_DPAD_LEFT_COL;
            break;
        case AZ_DPAD_DIR_RIGHT:
        default:
            *row = AZ_DPAD_RIGHT_ROW;
            *col = AZ_DPAD_RIGHT_COL;
            break;
    }
}

void az1uball_dpad_master_task(void) {
    if (!is_keyboard_master()) return;

    static uint32_t last_poll = 0;
    if (timer_elapsed32(last_poll) < AZ_DPAD_POLL_INTERVAL_MS) return;
    last_poll = timer_read32();

    az_dpad_sync_t recv = {0};
    if (!transaction_rpc_exec(USER_SYNC_AZ_DPAD, 0, NULL, sizeof(recv), &recv)) {
        return; // 通信失敗時は今回のポーリングをスキップ(次回また試す)
    }

    if (recv.has_direction) {
        uint8_t row, col;
        direction_to_rowcol((az_dpad_direction_t)recv.direction, &row, &col);
        fire_virtual_key(row, col);
    }

    static bool prev_click = false;
    if (recv.click != prev_click) {
        if (recv.click) {
            register_code(AZ_CLICK_KEYCODE);
        } else {
            unregister_code(AZ_CLICK_KEYCODE);
        }
        prev_click = recv.click;
    }
}

void az1uball_dpad_register_rpc(void) {
    transaction_register_rpc(USER_SYNC_AZ_DPAD, rpc_az_dpad_handler);
}