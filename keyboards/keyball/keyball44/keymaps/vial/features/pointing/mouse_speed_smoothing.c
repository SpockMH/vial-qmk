#include "mouse_speed_smoothing.h"
#include "features/config/eeconfig_user.h"

__attribute__((weak)) uint16_t SPD_THL_UP_KEYCODE = KC_NO;
__attribute__((weak)) uint16_t SPD_THL_DN_KEYCODE = KC_NO;
__attribute__((weak)) uint16_t SPD_THU_UP_KEYCODE = KC_NO;
__attribute__((weak)) uint16_t SPD_THU_DN_KEYCODE = KC_NO;
__attribute__((weak)) uint16_t SPD_SCL_UP_KEYCODE = KC_NO;
__attribute__((weak)) uint16_t SPD_SCL_DN_KEYCODE = KC_NO;

#define SPEED_BUF_SIZE  10
#define THRESHOLD_STEP  1   // +/-キー1回あたりの閾値の増減幅
#define SCALE_STEP_PCT  5   // +/-キー1回あたりの倍率の増減幅(%)
#define THRESHOLD_MAX   1000

// ---- リングバッファ(直近10回分の移動量の大きさを合計で保持) ----
typedef struct {
    int16_t buf[SPEED_BUF_SIZE];
    uint8_t idx;
    int32_t sum; // 常に現在の合計値を保持(差分更新でO(1))
} speed_buf_t;

static speed_buf_t speed_buf = {0};

// 直近 mouse_speed_smoothing_apply() で実際に使われた倍率(%)。OLED表示用。
static uint8_t last_scale_pct = 100;

void mouse_speed_smoothing_push(int16_t magnitude) {
    speed_buf.sum -= speed_buf.buf[speed_buf.idx];
    speed_buf.buf[speed_buf.idx] = magnitude;
    speed_buf.sum += magnitude;
    speed_buf.idx = (speed_buf.idx + 1) % SPEED_BUF_SIZE;
}

// ---- 台形カーブの設定(+/-キーで実行時に調整可能、EEPROMにも永続化される) ----
typedef struct {
    int16_t lower_threshold; // これ以下は min_scale_pct 固定("_"の部分)
    int16_t upper_threshold; // これ以上は 100%("←"の先、平坦部分)
    uint8_t min_scale_pct;   // 下限側の最小倍率(%)
} speed_smooth_config_t;

static speed_smooth_config_t cfg = {
    .lower_threshold = 10,
    .upper_threshold = 40,
    .min_scale_pct   = 50,
};

int16_t mouse_speed_smoothing_apply(int16_t v) {
    int32_t sum = speed_buf.sum;
    int32_t scale_pct;

    if (sum <= cfg.lower_threshold) {
        // 下限以下: 固定の最小倍率("_"の平坦部分)
        scale_pct = cfg.min_scale_pct;
    } else if (sum >= cfg.upper_threshold) {
        // 上限以上: 等倍("←"の先、平坦部分)
        scale_pct = 100;
    } else {
        // 中間: 線形補間("/"の傾斜部分)
        int32_t range = cfg.upper_threshold - cfg.lower_threshold;
        if (range <= 0) range = 1; // ゼロ割回避(通常は起きない想定)
        scale_pct = cfg.min_scale_pct + ((100 - cfg.min_scale_pct) * (sum - cfg.lower_threshold)) / range;
    }

    last_scale_pct = (uint8_t)scale_pct;

    int16_t result = (int16_t)(((int32_t)v * scale_pct) / 100);

    // vが0でないのに、整数演算の丸めでresultが0になってしまうケースをガードする。
    // 例: v=1, scale_pct=50 → 1*50/100=0(切り捨て)。これをそのまま返すと
    // 低速時に微小な動きが完全に消え、カーソルが反応しなくなってしまう。
    if (v != 0 && result == 0) {
        result = (v > 0) ? 1 : -1;
    }

    return result;
}

// ---- 調整キー処理 ----
bool process_mouse_speed_smoothing(uint16_t keycode, keyrecord_t *record) {
    if (!record->event.pressed) return true;

    if (keycode == SPD_THL_UP_KEYCODE) {
        cfg.lower_threshold += THRESHOLD_STEP;
        // 下限が上限に食い込まないようにクリップ
        if (cfg.lower_threshold > cfg.upper_threshold - THRESHOLD_STEP) {
            cfg.lower_threshold = cfg.upper_threshold - THRESHOLD_STEP;
        }
        return false;
    }
    if (keycode == SPD_THL_DN_KEYCODE) {
        cfg.lower_threshold -= THRESHOLD_STEP;
        if (cfg.lower_threshold < 0) cfg.lower_threshold = 0;
        return false;
    }
    if (keycode == SPD_THU_UP_KEYCODE) {
        cfg.upper_threshold += THRESHOLD_STEP;
        if (cfg.upper_threshold > THRESHOLD_MAX) cfg.upper_threshold = THRESHOLD_MAX;

        return false;
    }
    if (keycode == SPD_THU_DN_KEYCODE) {
        cfg.upper_threshold -= THRESHOLD_STEP;
        // 上限が下限を割り込まないようにクリップ
        if (cfg.upper_threshold < cfg.lower_threshold + THRESHOLD_STEP) {
            cfg.upper_threshold = cfg.lower_threshold + THRESHOLD_STEP;
        }
        
        return false;
    }
    if (keycode == SPD_SCL_UP_KEYCODE) {
        cfg.min_scale_pct += SCALE_STEP_PCT;
        if (cfg.min_scale_pct > 100) cfg.min_scale_pct = 100;
        
        return false;
    }
    if (keycode == SPD_SCL_DN_KEYCODE) {
        if (cfg.min_scale_pct >= SCALE_STEP_PCT) {
            cfg.min_scale_pct -= SCALE_STEP_PCT;
        } else {
            cfg.min_scale_pct = 0;
        }
        
        return false;
    }

    return true;
}

// ---- OLED表示用のgetter ----
int16_t mouse_speed_smoothing_get_lower_threshold(void) {
    return cfg.lower_threshold;
}

int16_t mouse_speed_smoothing_get_upper_threshold(void) {
    return cfg.upper_threshold;
}

uint8_t mouse_speed_smoothing_get_min_scale_pct(void) {
    return cfg.min_scale_pct;
}

int32_t mouse_speed_smoothing_get_current_sum(void) {
    return speed_buf.sum;
}

uint8_t mouse_speed_smoothing_get_current_scale_pct(void) {
    return last_scale_pct;
}

// ---- EEPROM永続化用(eeconfig_user.cから呼ぶ) ----
void mouse_speed_smoothing_set_config(int16_t lower_threshold, int16_t upper_threshold, uint8_t min_scale_pct) {
    cfg.lower_threshold = lower_threshold;
    cfg.upper_threshold = upper_threshold;
    cfg.min_scale_pct   = min_scale_pct;
}

void mouse_speed_smoothing_get_config(int16_t *lower_threshold, int16_t *upper_threshold, uint8_t *min_scale_pct) {
    *lower_threshold = cfg.lower_threshold;
    *upper_threshold = cfg.upper_threshold;
    *min_scale_pct   = cfg.min_scale_pct;
}