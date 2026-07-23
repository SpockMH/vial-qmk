#include QMK_KEYBOARD_H
#include "quantum.h"
#include <lib/lib8tion/lib8tion.h>
#include <stdlib.h>
#include "split_util.h"
#include "transactions.h"
#include "features/rgblight_user.h"
#include "features/mouse_mode.h"
#include "features/jis2us.h"
#include "features/eeconfig_user.h"
#include "features/oled_user.h"
#include "features/arrow_layer.h"
#include "features/az1uball_gesture.h"
#include "features/select_extend.h"
#include "features/mouse_speed_smoothing.h"
#include "features/virtual_key.h"

static uint16_t move_timer;
// ball_tension は arrow_layer.c 内部のテンション管理に統合したため削除
const  uint8_t SCR_layer = 4; //スクロールのレイヤー

#define RGB_IDLE_TIMEOUT_MS 30000
static uint32_t last_rgb_activity = 0;
static bool     rgb_is_idle       = false;

//左右同期用のデータ構造
typedef struct _sync_data_t{
  //キーのrow,col位置情報(ライティング用)
  uint8_t key_row:3;
  uint8_t key_col:3;
  bool scr:1;
  bool click:1;
} sync_data_t;

sync_data_t sync_data = {0,0,false,false};

bool splash_mode=false;
bool arrow_key_mode = false;

enum custom_keycodes {
    CPI_x3 = KEYBALL_SAFE_RANGE, // Keyballのセーフレンジを起点にする
    JPUS_TOG,
    MMD_TOG,                    
    MMT_DEC,                    
    MMT_INC,
    LIGHT_TOG,
    LIGHT_VAI,
    LIGHT_VAD,
    HUE_UP,
    SPL_TOG,
    SELWORD_LEFT,
    SELWORD_RIGHT,
    SELLINE_UP,
    SELLINE_DOWN,
    SPD_THL_UP,
    SPD_THL_DN,
    SPD_THU_UP,
    SPD_THU_DN,
    SPD_SCL_UP,
    SPD_SCL_DN,
};

uint16_t SELECT_WORD_LEFT_KEYCODE  = SELWORD_LEFT;
uint16_t SELECT_WORD_RIGHT_KEYCODE = SELWORD_RIGHT;
uint16_t SELECT_LINE_UP_KEYCODE    = SELLINE_UP;
uint16_t SELECT_LINE_DOWN_KEYCODE  = SELLINE_DOWN;
uint16_t SPD_THL_UP_KEYCODE = SPD_THL_UP;
uint16_t SPD_THL_DN_KEYCODE = SPD_THL_DN;
uint16_t SPD_THU_UP_KEYCODE = SPD_THU_UP;
uint16_t SPD_THU_DN_KEYCODE = SPD_THU_DN;
uint16_t SPD_SCL_UP_KEYCODE = SPD_SCL_UP;
uint16_t SPD_SCL_DN_KEYCODE = SPD_SCL_DN;


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

///////////////////////////////////////////////////////////////////////////////
//SLAVE側で発生する処理
///////////////////////////////////////////////////////////////////////////////
void user_sync_a_update_keyCounter_on_other_board(uint8_t in_buflen, const void* in_data, uint8_t out_buflen, void* out_data) {
  const sync_data_t* data = (const sync_data_t*)in_data;
  rgblight_value(data->key_row,data->key_col,true,data->scr,data->click);
}

static void lighting_sync_slave_handler(uint8_t in_buflen, const void *in_data,
                                        uint8_t out_buflen, void *out_data) {
    rgblight_update_sync((rgblight_simple_config_t *)in_data);
}

static void call_rgblight(bool scr){
  sync_data.scr = scr;
  sync_data.click = false;
  //更新された値でライティング更新
  rgblight_value(sync_data.key_row , sync_data.key_col,true,scr,false);
  //右側のROW,COL を左側に変換したうえで同期
  sync_data.key_row = sync_data.key_row - 4;
  sync_data.key_col = 5 - sync_data.key_col;
  if(is_keyboard_master()){ 
    transaction_rpc_send(USER_SYNC_KEY_COUNTER, sizeof(sync_data), &sync_data);
  }
}

static void wake_rgb(void) {
    last_rgb_activity = timer_read32();
    if (rgb_is_idle) {
        rgblight_enable();
        rgb_is_idle = false;
    }
}

void keyboard_post_init_user(void) {
  transaction_register_rpc(USER_SYNC_KEY_COUNTER, user_sync_a_update_keyCounter_on_other_board);
  transaction_register_rpc(USER_SYNC_LIGHTING, lighting_sync_slave_handler);
  user_config_init();
  last_rgb_activity = timer_read32();
  rgblight_init();

  process_arrow_layer_reset();
}

void matrix_scan_user(void) {
  bool click = mouse_mode_scan();
  if (click){
    sync_data.click  = true;
    rgblight_value(sync_data.key_row , sync_data.key_col,true,false,true);
    if(is_keyboard_master()){ 
      transaction_rpc_send(USER_SYNC_KEY_COUNTER, sizeof(sync_data), &sync_data);
    }
  }
}


void housekeeping_task_user(void) {
  rgblight_task();
  if (!rgb_is_idle &&
    timer_elapsed32(last_rgb_activity) > RGB_IDLE_TIMEOUT_MS) {
    rgblight_disable();
    rgb_is_idle = true;
  }
}


///////////////////////////////////////////////////////////////////////////////
//キー処理
///////////////////////////////////////////////////////////////////////////////
// CPI_x3を押している間だけCPIを3倍にする処理
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
static bool process_cpi_x3(uint16_t keycode, keyrecord_t *record) {
  if (keycode != CPI_x3) return true;
  if (record->event.pressed) {
    keyball_set_cpi(keyball_get_cpi()*3);
  } else {
    keyball_set_cpi(keyball_get_cpi()/3);
  }
  return false;
}

// このキーボード独自のユーザー定義キーコード（押下時のみ処理するもの）
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
static bool process_user_keycode(uint16_t keycode, keyrecord_t *record) {
  if (!record->event.pressed) return true;
  switch (keycode) {
    case JPUS_TOG:
        jis2us_toggle();
        return false;
 
    case MMD_TOG:
        mouse_mode_toggle(); // 関数を呼び出す
        return false;
 
    case MMT_DEC:
        mouse_mode_change_term(false);
        return false;
 
    case MMT_INC:
        mouse_mode_change_term(true);
        return false;
 
    case LIGHT_TOG:
        return false;
 
    case LIGHT_VAI:
        rgblight_increase_val();
        return false;
 
    case LIGHT_VAD:
        rgblight_decrease_val();
        return false;
 
    case HUE_UP:
        set_hue(get_hue() + 3);
        return false;

    case SPL_TOG:
        splash_mode = !splash_mode;
        return false;
        
    case KBC_SAVE:
        save_user_config();
  }
  return true;
}

// ユーザー定義キーコードを処理するためのディスパッチ関数。
// virtual_key.c の fire_virtual_key() から、0x7E00以降のキーコードに対して呼ばれる。
bool process_user_virtual_keycode(uint16_t keycode) {
  switch (keycode) {
    case SELWORD_LEFT:
      select_extend_word_left();
      return false;
    case SELWORD_RIGHT:
      select_extend_word_right();
      return false;
    case SELLINE_UP:
      select_extend_line_up();
      return false;
    case SELLINE_DOWN:
      select_extend_line_down();
      return false;

    case QK_TOGGLE_LAYER ... QK_TOGGLE_LAYER_MAX: {
      // キーコードからレイヤー番号 (0〜31) を抽出
      uint8_t layer = QK_TOGGLE_LAYER_GET_LAYER(keycode);
      
      if (layer_state_is(layer)) {
        layer_off(layer);
      } else {
        layer_on(layer);
      }
      return false;
    }
    default:
      return true; // 未対応のキーコードは無視
  }
}

// 日本語入力(IME)のかな/英数状態に追従して、変換確定後に自動でIME ON(かな)へ戻す処理。
// KC_LEFT_BRACKETでIME ONを明示的に叩いた直後の状態もここでまとめて管理する。
// 戻り値: true=以降の処理を継続, false=ここで処理済みとして打ち切り
static bool process_kana_ime(uint16_t keycode, keyrecord_t *record) {
  static bool kana = false;
  
  if (keycode == KC_INTERNATIONAL_5) {
    kana = false;
  } else if (keycode == KC_INTERNATIONAL_4) {
    kana = true;
  }
 
  // 変換候補ウィンドウ操作のキーコード範囲。離した時にkana状態ならIME ONへ戻す。
  if (keycode >= 0x7700 && keycode <= 0x7703) {
    if (!record->event.pressed && kana) {
      tap_code16(KC_INTERNATIONAL_4);
    }
  }
 
  if (keycode == KC_LEFT_BRACKET) {
    if (record->event.pressed) {
      tap_code16(KC_INTERNATIONAL_5);
      return true;
    }
  }
 
  return true;
}


bool process_record_user(uint16_t keycode, keyrecord_t *record){  
  //キーイベント情報をSLAVE側に同期
  move_timer = timer_read();
  if(record->event.pressed){
    wake_rgb();
    sync_data.key_row = record->event.key.row;
    sync_data.key_col = record->event.key.col;
    sync_data.click = splash_mode;
    transaction_rpc_send(USER_SYNC_KEY_COUNTER, sizeof(sync_data), &sync_data);
    //MASTER側のライティング処理
    rgblight_value(sync_data.key_row , sync_data.key_col,true,false,splash_mode);
  }

  if (!process_cpi_x3(keycode, record)) return false;
  if (!process_select_extend(keycode, record)) return false;
  if (!process_user_keycode(keycode, record)) return false;
  if (record->event.pressed){
    if (!process_user_virtual_keycode(keycode)) return false;
  }
  if (!process_kana_ime(keycode, record)) return false;
  if (!jis2us_process(keycode, record)) return false;
  if (!process_mouse_speed_smoothing(keycode, record)) return false;
  return mouse_mode_process(keycode, record);

}

///////////////////////////////////////////////////////////////////////////////
//マウス処理
///////////////////////////////////////////////////////////////////////////////
// ---- pointing_device_task_user で使う定数 ----
#define ARROW_LAYER_TENSION_THRESHOLD 30   // layer1/2で矢印/BSDELキーを発火させ始めるテンション量

#define LIGHTING_TENSION_THRESHOLD    50   // ライティング座標(row/col)を1マス進めるテンション量
#define LIGHTING_V_DIV                2    // mouse_report.y を垂直テンションに加算する際の除数
#define LIGHTING_SCROLL_V_MULT        4    // スクロール量(v)を垂直テンションへ変換する係数
#define LIGHTING_H_DIV                2    // mouse_report.x を水平テンションに加算する際の除数

// ライティング座標(row/col)管理専用のテンション。
// process_arrow_layer用のテンションはfeatures/arrow_layer.cに移動した。
static int16_t lighting_tension_v = 0;
static int16_t lighting_tension_h = 0;

// マウスポインタの動作量をライティング用のRow/Col座標(sync_data)に変換し、
// 一定テンションを超えたら座標を1マス進めて同期する処理。
static void process_lighting_tracking(report_mouse_t *mouse_report, bool layerscr) {
  //ROWが左側だった場合、右側に変更(右側の情報を最後に左に変換し同期)
  if(sync_data.key_row < 4){
    sync_data.key_row = sync_data.key_row + 4;
    sync_data.key_col = 5 - sync_data.key_col;
  }
  lighting_tension_h += mouse_report->x / LIGHTING_H_DIV;
  lighting_tension_v += mouse_report->y / LIGHTING_V_DIV - mouse_report->v * LIGHTING_SCROLL_V_MULT;
 
  bool move = false;
  //テンションが一定以上であれば上下左右のキー座標変更処理
  if(lighting_tension_h > LIGHTING_TENSION_THRESHOLD){
    if(sync_data.key_col!=0){
      sync_data.key_col = sync_data.key_col - 1;
    }
    lighting_tension_h = 0;
    move = true;
  } else if (lighting_tension_h < -LIGHTING_TENSION_THRESHOLD){
    if(sync_data.key_col!=5){
      sync_data.key_col = sync_data.key_col + 1;
    }
    lighting_tension_h = 0;
    move = true;
  }
  if(lighting_tension_v > LIGHTING_TENSION_THRESHOLD){
    if(sync_data.key_row!=6){
      sync_data.key_row = sync_data.key_row + 1;
    } else if (layerscr){
      sync_data.key_row = 4;
    }
    lighting_tension_v =0;
    move = true;
  } else if (lighting_tension_v < -LIGHTING_TENSION_THRESHOLD){
    if(sync_data.key_row!=4){
      sync_data.key_row = sync_data.key_row - 1;
    } else if (layerscr){
      sync_data.key_row = 6;
    }
    lighting_tension_v =0;
    move = true;
  }
  //キー座標が変更となっていた場合にライティング情報変更及び同期処理
  if(move){
    clear_m_buf();
    call_rgblight(layerscr);
    move_timer = timer_read();
  }
}

report_mouse_t pointing_device_task_user(report_mouse_t mouse_report) {

    // M-MODE用のモーション量計算（MOUSE_EXTENDED_REPORTでx/yはint16_t）
    int16_t x = mouse_report.x;
    int16_t y = mouse_report.y;
    uint16_t abs_move = abs(x) + abs(y);

    mouse_move(abs_move > 1);

    bool layer1      = layer_state_is(1);
    bool layer2      = layer_state_is(2);
    bool scroll_mode = keyball_get_scroll_mode(); 

    static bool az_btn_prev = false;
    bool az_btn_now = (mouse_report.buttons & MOUSE_BTN6);
    
    bool az_press_edge = az_btn_now && !az_btn_prev;  // 押下エッジ検出
    if (az_press_edge) {
        sync_data.click  = true;
        rgblight_value(2,5,true,false,true);
        if(is_keyboard_master()){ 
          transaction_rpc_send(USER_SYNC_KEY_COUNTER, sizeof(sync_data), &sync_data);
        }
    }

    if (scroll_mode) {
        process_az1uball_gesture(mouse_report.x,mouse_report.y,az_press_edge);
        mouse_report.x = 0;
        mouse_report.y = 0;
    } else {
        process_az1uball_gesture(mouse_report.h,-mouse_report.v,az_press_edge);
        mouse_report.h = 0;
        mouse_report.v = 0;
    }

    az_btn_prev   = az_btn_now;

    // ----------------------------------------------------------------
    // layer1/2: PMW3360(右手)の動作量を矢印キー/BSDEL・DELに変換する。
    //   layer1 → 左右のみ (KC_BSPC/KC_DEL)、上下は無効(KC_NO)
    //   layer2 → 上下左右 (矢印キー)
    // テンション管理は arrow_layer.c 内部で完結。
    // ----------------------------------------------------------------
    if (layer1 || layer2) {
        if (layer1) {
            process_arrow_layer(x, y, KC_NO, KC_NO, KC_BSPC, KC_DEL);
        } else {
            process_arrow_layer(x, y, KC_UP, KC_DOWN, KC_LEFT, KC_RIGHT);
        }
        mouse_report.x = 0;
        mouse_report.y = 0;
        return mouse_report;
    }

    process_lighting_tracking(&mouse_report, scroll_mode);

    mouse_speed_smoothing_push(abs_move);
    mouse_report.x = mouse_speed_smoothing_apply(mouse_report.x);
    mouse_report.y = mouse_speed_smoothing_apply(mouse_report.y);

,

    return mouse_report;
}


///////////////////////////////////////////////////////////////////////////////
//レイヤー変更処理
///////////////////////////////////////////////////////////////////////////////
layer_state_t layer_state_set_user(layer_state_t state)
{

  uint8_t layer = get_highest_layer(state);
  
  //レイヤー4の場合スクロールモードに変更
  keyball_set_scroll_mode(layer == SCR_layer);

  // レイヤー切替時にテンションをリセット(切替前の蓄積が誤発火するのを防ぐ)
  process_arrow_layer_reset();

  
  if(layer==1){
    //layer1,テンキーレイヤーを有効にするため、numlockもONにする。
    if (!host_keyboard_led_state().num_lock){
      tap_code(0x53);
    }
    rgblight_mode(RGBLIGHT_MODE_ICEWAVE);

  } else if (layer== 2){
    rgblight_mode(RGBLIGHT_MODE_STATIC);

  } else if (layer== SCR_layer){
    rgblight_mode(RGBLIGHT_MODE_SCROLLMOVE);
    arrow_key_mode = false;
    call_rgblight(true);


  } else {
    rgblight_mode(RGBLIGHT_MODE_MOUSEMOVE);
    call_rgblight(false);

  }
  return state;
}

