#include QMK_KEYBOARD_H
#include "quantum.h"
#include <lib/lib8tion/lib8tion.h>
#include <stdlib.h>
#include "split_util.h"

// lighting/ : RGBライティング制御
#include "features/lighting/rgblight_user.h"
#include "features/lighting/lighting_tracking.h"

// pointing/ : トラックボール入力処理(モーション→ジェスチャー/キー変換、速度調整)
#include "features/pointing/mouse_mode.h"
#include "features/pointing/mouse2key.h"
#include "features/pointing/az1uball_gesture.h"
#include "features/pointing/mouse_speed_smoothing.h"
#include "features/pointing/virtual_key.h"

// key_control/ : キーコード定義・変換・ディスパッチ系
#include "features/key_control/custom_keycodes.h"
#include "features/key_control/jis2us.h"
#include "features/key_control/select_extend.h"
#include "features/key_control/utils.h"

// display/ : OLED表示
#include "features/display/oled_user.h"

// config/ : EEPROM永続化
#include "features/config/eeconfig_user.h"

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
  // keymap for default
  [0] = LAYOUT(
    KC_ESC , KC_Q   , KC_W   , KC_E   , KC_R   , KC_T   ,                      KC_Y   , KC_U   , KC_I   , KC_O   , KC_P   , KC_EQUAL,
    0x222B , KC_A   , KC_S   , 0x2207 , 0x2409 , KC_G   ,                      KC_H   , KC_J   , 0x440E , KC_L   , 0x422D , KC_ENTER,
    0x7E40 , 0x221D , KC_X   , KC_C   , 0x2819 , KC_B   ,                      KC_N   , KC_M   , KC_COMM, KC_DOT , KC_SLSH, 0x4287  ,
             _______,  KC_LGUI, KC_LALT, 0x2150 , 0x412C , 0x424F ,,     0x312A ,0x324C   ,_______,  _______,          0X5261, _______
  ),	

  [1] = LAYOUT(
    0X43D  , S(KC_1), S(KC_2), S(KC_3), S(KC_4), S(KC_5),                      0x22D  , KC_KP_7, KC_KP_8, KC_KP_9, 0x22E  , 0x2F   ,
    _______, S(KC_6), S(KC_7), 0X2233 , 0x2434 , KC_UP  ,                      0x57   , KC_KP_4, KC_KP_5, KC_KP_6, 0x4456 , _______,
    _______, 0x11D  , 0x11B  , 0x106  , 0x119  , KC_DOWN,                      0x55   , KC_KP_1, KC_KP_2, KC_KP_3, 0x54   , 0x289  ,
            _______,  _______, _______, _______, _______, _______,,    0x2163 , 0x3262  , _______,  _______,        _______, _______
  ),

  [2] = LAYOUT(
    _______,KC_F9   , KC_F10 , KC_F11 , KC_F12 , 0x32   ,                      _______, 0x7E41 , KC_UP  , KC_NO  , KC_NO  , _______,
    0x868  ,KC_F5   , KC_F6  , 0x2240 , KC_F8  , 0x4B   ,                      _______, KC_LEFT, KC_DOWN,KC_RIGHT, KC_NO  , _______,
    _______,KC_F1   , KC_F2  , KC_F3  , KC_F4  , 0x4E   ,                      _______, KC_NO  , KC_NO  , KC_NO  , 0x7C01 , _______,
            _______,  QK_BOOT, 0x7705 , _______, _______, _______,,   _______, _______  ,_______,_______,           QK_BOOT,_______
  ),

  [3] = LAYOUT(
    _______, _______, _______, _______, _______, _______,                      _______, _______, _______, _______, _______, _______,
    _______, _______, _______, _______, _______, _______,                      _______, 0xD1   , _______, 0xD2   , 0x5080, _______,
    _______, _______, _______, _______, _______, _______,                      _______, _______, _______, _______, _______, _______,
             _______, _______, _______, _______, _______, _______,,    _______, _______ , _______,_______,        _______, _______
  ),

  [4] = LAYOUT(
    _______, _______, 0x950  , 0x852  , 0x94F  , _______,                      QK_KB_2, 0x424  , _______, 0x425  , _______, _______,
    _______, 0xA50  , 0x850  , 0x851  , 0x84F  , 0xA4F  ,                      0x326  , 0xD1   , _______, 0xD2   , _______, _______,
    _______, _______, 0x32D  , _______, 0x333  , _______,                      QK_KB_3, _______, _______, _______, _______, _______,
            _______, _______, _______, _______, _______, _______,,    0xD4   , 0xD5    , _______,  _______,         QK_KB_1, _______
  ),
};

void keyboard_post_init_user(void) {
  oled_status_sync_register();
  user_config_init();
  rgblight_init();
  lighting_tracking_init();
}

void matrix_scan_user(void) {
  bool click = mouse_mode_scan();
  if (click){
    lighting_tracking_trigger(false, true);
  }
}

void housekeeping_task_user(void) {
  rgblight_task();
  oled_status_sync_task();
}

///////////////////////////////////////////////////////////////////////////////
//キー処理
///////////////////////////////////////////////////////////////////////////////
bool process_record_user(uint16_t keycode, keyrecord_t *record){  
  
  if(record->event.pressed){
    // rgblight関係処理
    rgblight_wake();
    lighting_tracking_set_position(record->event.key.row, record->event.key.col);
    lighting_tracking_trigger(false, false);
  }

  if (!process_cpi_x3(keycode, record)) return false;
  if (!process_select_extend(keycode, record)) return false;
  if (!process_user_keycode(keycode, record)) return false;
  if (!process_kana_ime(keycode, record)) return false;
  if (!jis2us_process(keycode, record)) return false;
  if (!process_mouse_speed_smoothing(keycode, record)) return false;
  return mouse_mode_process(keycode, record);

}

///////////////////////////////////////////////////////////////////////////////
//マウス処理
///////////////////////////////////////////////////////////////////////////////
report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {

    // M-MODE用のモーション量計算（MOUSE_EXTENDED_REPORTでx/yはint16_t）
    int16_t x = mouse_report.x;
    int16_t y = mouse_report.y;
    uint16_t abs_move = abs(x) + abs(y);

    mouse_move(abs_move > 1);

    bool scroll_mode = keyball_get_scroll_mode();

    // layer1/2: PMW3360(右手)の動作量を矢印キー/BSDEL・DELに変換する。
    // layer1 → 左右のみ (KC_BSPC/KC_DEL)、上下は無効(KC_NO)
    // layer2 → 上下左右 (矢印キー)
    if (layer_state_is(1)) process_mouse2key(&mouse_report, KC_NO, KC_NO, KC_BSPC, KC_DEL);
    if (layer_state_is(2)) process_mouse2key(&mouse_report, KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT);

    handle_az1uball_input(&mouse_report, scroll_mode);
    lighting_tracking_update(&mouse_report, scroll_mode);
    mouse_speed_smoothing(&mouse_report);

    return mouse_report;
}


///////////////////////////////////////////////////////////////////////////////
//レイヤー変更処理
///////////////////////////////////////////////////////////////////////////////
layer_state_t layer_state_set_user(layer_state_t state)
{
    uint8_t layer = get_highest_layer(state);

    // レイヤー4 (スクロール用レイヤー) の場合スクロールモードに変更
    keyball_set_scroll_mode(layer == 4);

    // レイヤー切替時にテンションをリセット (切替前の蓄積が誤発火するのを防ぐ)
    process_mouse2key_reset();

    switch (layer) {
        case 1:
            // テンキーレイヤー有効化に伴い NumLock も ON にする
            if (!host_keyboard_led_state().num_lock) tap_code(KC_NUM_LOCK);
            rgblight_mode(RGBLIGHT_MODE_ICEWAVE);
            break;

        case 2:
            rgblight_mode(RGBLIGHT_MODE_STATIC);
            break;

        case 3:
            rgblight_mode(RGBLIGHT_MODE_SWIRL);
            break;

        case 4:
            rgblight_mode(RGBLIGHT_MODE_SCROLLMOVE);
            lighting_tracking_set_position(5, 4);
            lighting_tracking_trigger(true, false);
            break;

        case 5:
            rgblight_mode(RGBLIGHT_MODE_CROSS);
            break;
        default:
            rgblight_mode(RGBLIGHT_MODE_MOUSEMOVE);
            lighting_tracking_set_position(5, 4);
            lighting_tracking_trigger(false, false);
            break;
    }

    return state;
}