#include "lighting_tracking.h"
#include "rgblight_user.h"
#include "features/pointing/mouse_mode.h"
#include "transactions.h"

#define TENSION_THRESHOLD 50 // 座標を1マス進めるのに必要なテンション量
#define V_DIV              2 // mouse_report.y -> 垂直テンションの除数
#define SCROLL_V_MULT       4 // スクロール量(v) -> 垂直テンションへの係数
#define H_DIV               2 // mouse_report.x -> 水平テンションの除数

// 左右同期用のデータ構造(元keymap.cのsync_data_tと同一レイアウト)
typedef struct _sync_data_t {
    uint8_t key_row : 3;
    uint8_t key_col : 3;
    bool    scr : 1;
    bool    click : 1;
} sync_data_t;

static sync_data_t sync_data = {0, 0, false, false};
static int16_t     tension_v = 0;
static int16_t     tension_h = 0;

void lighting_tracking_init(void) {
    sync_data = (sync_data_t){0, 0, false, false};
    tension_v = 0;
    tension_h = 0;
}

void lighting_tracking_set_position(uint8_t row, uint8_t col) {
    sync_data.key_row = row;
    sync_data.key_col = col;
}

// 右手側基準の座標をスレーブ(左手側)向けに変換して同期送信する。
static void sync_to_slave(void) {
    sync_data.key_row = sync_data.key_row - LIGHTING_ROW_OFFSET;
    sync_data.key_col = LIGHTING_COL_MAX - sync_data.key_col;
    if (is_keyboard_master()) {
        transaction_rpc_send(USER_SYNC_KEY_COUNTER, sizeof(sync_data), &sync_data);
    }
}

void lighting_tracking_trigger(bool scr, bool click) {
    sync_data.scr   = scr;
    sync_data.click = click;
    rgblight_value(sync_data.key_row, sync_data.key_col, true, scr, click);
    sync_to_slave();
}

void lighting_tracking_rpc_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    const sync_data_t *data = (const sync_data_t *)in_data;
    rgblight_value(data->key_row, data->key_col, true, data->scr, data->click);
}

void lighting_tracking_update(report_mouse_t *mouse_report, bool layerscr) {
    // 左手側の座標が残っていたら右手側基準へ正規化
    // (ROWが左側だった場合、右側に変更。右側の情報を最後に左に変換し同期する)
    if (sync_data.key_row < LIGHTING_ROW_OFFSET) {
        sync_data.key_row = sync_data.key_row + LIGHTING_ROW_OFFSET;
        sync_data.key_col = LIGHTING_COL_MAX - sync_data.key_col;
    }

    tension_h += mouse_report->x / H_DIV;
    tension_v += mouse_report->y / V_DIV - mouse_report->v * SCROLL_V_MULT;

    bool moved = false;

    // テンションが一定以上であれば上下左右のキー座標変更処理
    if (tension_h > TENSION_THRESHOLD) {
        if (sync_data.key_col != 0) sync_data.key_col = sync_data.key_col - 1;
        tension_h = 0;
        moved     = true;
    } else if (tension_h < -TENSION_THRESHOLD) {
        if (sync_data.key_col != LIGHTING_COL_MAX) sync_data.key_col = sync_data.key_col + 1;
        tension_h = 0;
        moved     = true;
    }

    if (tension_v > TENSION_THRESHOLD) {
        if (sync_data.key_row != 6) {
            sync_data.key_row = sync_data.key_row + 1;
        } else if (layerscr) {
            sync_data.key_row = 4; // スクロールレイヤー中は 4-6 の範囲でラップ
        }
        tension_v = 0;
        moved     = true;
    } else if (tension_v < -TENSION_THRESHOLD) {
        if (sync_data.key_row != 4) {
            sync_data.key_row = sync_data.key_row - 1;
        } else if (layerscr) {
            sync_data.key_row = 6;
        }
        tension_v = 0;
        moved     = true;
    }

    // キー座標が変更となっていた場合にライティング情報変更及び同期処理
    if (moved) {
        clear_m_buf();
        lighting_tracking_trigger(layerscr, false);
    }
}
