/*
 * az1uball.c  —  AZ1UBALL I2C トラックボール ドライバ (QMK / RP2040)
 *
 * プロトコル仕様 (公式ファームウェア tiny424_trackball.ino より):
 *   アドレス: 0x0A (固定)
 *   読み出し: レジスタ指定なし。Wire.requestFrom 相当で 5 バイト受信。
 *   バイト構成:
 *     [0] 左移動量  (uint8_t, 累積加算値。読み出し後ゼロリセット)
 *     [1] 右移動量  (uint8_t)
 *     [2] 上移動量  (uint8_t)
 *     [3] 下移動量  (uint8_t)
 *     [4] ボタン    (0x80=押下, 0x00=離)
 *
 *   カウントモード設定 (初期化時に1バイト送信):
 *     0x90 → 通常モード (常に1加算)
 *     0x91 → AZ用モード (速度に応じて1/2/3加算) ← デフォルト
 */

#include "az1uball.h"
#include "i2c_master.h"
#include "debug.h"

static bool az1uball_ready = false;

bool az1uball_init(void) {
    // i2c_init() はQMKコア(OLED等)がすでに呼んでいるため、ここでは呼ばない。
    // 右手側でi2c_init()を再呼び出しするとOLED用I2Cバスが再初期化されて壊れる。

    // カウントモードを設定 (0x90=通常 / 0x91=AZ用)
    uint8_t mode = (AZ1UBALL_COUNT_MODE == 0) ? 0x90 : 0x91;
    i2c_status_t status = i2c_transmit(
        AZ1UBALL_I2C_ADDR << 1,  // QMK は 8-bit アドレス形式
        &mode,
        1,
        AZ1UBALL_I2C_TIMEOUT
    );

    if (status != I2C_STATUS_SUCCESS) {
        dprintf("az1uball: init failed (status=%d)\n", status);
        az1uball_ready = false;
        return false;
    }

    az1uball_ready = true;
    dprintf("az1uball: init OK (addr=0x%02X, mode=0x%02X)\n", AZ1UBALL_I2C_ADDR, mode);
    return true;
}

bool az1uball_get_motion(int16_t *x, int16_t *y, bool *btn) {
    if (!az1uball_ready) return false;

    // レジスタ指定なしで 5 バイト受信
    uint8_t buf[5] = {0};
    i2c_status_t status = i2c_receive(
        AZ1UBALL_I2C_ADDR << 1,
        buf,
        5,
        AZ1UBALL_I2C_TIMEOUT
    );

    if (status != I2C_STATUS_SUCCESS) {
        dprintf("az1uball: receive failed (status=%d)\n", status);
        return false;
    }

    // buf[0]=左, [1]=右, [2]=上, [3]=下, [4]=ボタン
    // X: 右が正 → right - left
    // Y: 下が正 → down - up
    int16_t raw_x = (int16_t)buf[1] - (int16_t)buf[0];
    int16_t raw_y = (int16_t)buf[2] - (int16_t)buf[3];
    *btn = (buf[4] & 0x80) != 0;

    *x = raw_x * AZ1UBALL_SCALE;
    *y = raw_y * AZ1UBALL_SCALE;

    return (raw_x != 0 || raw_y != 0 || *btn);
}

report_mouse_t az1uball_get_report(report_mouse_t mouse_report) {
    int16_t x = 0, y = 0;
    bool    btn = false;
    az1uball_get_motion(&x, &y, &btn);

    // report_mouse_t.x/yはint8_t(MOUSE_EXTENDED_REPORT未定義時)なのでクリップする。
    // QMKコア(pointing_device_task)側で最終的にclip2int8相当の処理が入るが、
    // ここでも安全のため範囲内に収めておく。
    mouse_report.x = (x > 127) ? 127 : (x < -127 ? -127 : (int8_t)x);
    mouse_report.y = (y > 127) ? 127 : (y < -127 ? -127 : (int8_t)y);
    if (btn) {
        mouse_report.buttons |= MOUSE_BTN1;
    }
    return mouse_report;
}