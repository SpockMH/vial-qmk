#include "mouse_mode.h"

#define M_BUF_SIZE 3

#define LEFT_CLICK  KC_J
#define RIGHT_CLICK 0xFFFF
#define shift_key   0x2207
#define ctrl_key    0x2150
#define layer4_key  0x440E


// =============================
// Jバッファ
// =============================

typedef struct {
    uint8_t keycode;
    bool pressed;
} key_event_t;

static uint8_t M_TERM = 60;
static key_event_t m_buf[M_BUF_SIZE];
static uint8_t m_buf_len;

static uint16_t typing_timer;

static bool m_dragging;
static bool m_mode = true;

// =============================
// タイピング検知
// =============================

static bool latast_move_mouse;

// =============================
// 内部 util
// =============================

void clear_m_buf(void){
    m_buf_len = 0;
    m_buf[0].keycode = 0x0;
}

void mouse_move(bool move){
    if(move) {
        latast_move_mouse = true;
    }      
}

bool mouse_mode(void){
    return latast_move_mouse;
}

// =============================
// process_record_user 側
// =============================

bool inline mouse_mode_process(uint16_t keycode, keyrecord_t *record){

    static bool ctrl_or_shift_key_press = false;
    if( keycode == ctrl_key || keycode == shift_key || keycode == layer4_key){
        if (record->event.pressed){
            ctrl_or_shift_key_press = true;
        } else {
            ctrl_or_shift_key_press = false;
            return true;
        }
    }

    if(keycode != LEFT_CLICK && keycode != RIGHT_CLICK && !ctrl_or_shift_key_press){
        latast_move_mouse = false; 
    }

    // ドラッグ解除
    if(m_dragging && 
    (keycode == LEFT_CLICK || keycode == RIGHT_CLICK) && 
    !record->event.pressed){

        unregister_code(KC_MS_BTN1);
        unregister_code(KC_MS_BTN2);
        m_dragging = false;
        clear_m_buf();
        return false;
    }

    typing_timer =  timer_read();
    if (m_mode && (latast_move_mouse || m_buf_len>0)){
        // ===== Jイベント保存 =====
        if(m_buf_len < M_BUF_SIZE){
            m_buf[m_buf_len++] = (key_event_t){
                .keycode = keycode & 0xFF,
                .pressed = record->event.pressed
            };
        }

        // 先頭Jは一旦止める
        if(m_buf[0].keycode == LEFT_CLICK && record->event.pressed)
            return false;
        if(m_buf[0].keycode == RIGHT_CLICK && record->event.pressed)
            return false;

    }

    return true;
}

// =============================
// matrix_scan_user 側
// =============================

bool mouse_mode_scan(void){

    if (!m_mode || m_buf_len == 0) return false;

    if(timer_elapsed(typing_timer) < M_TERM && m_buf_len < M_BUF_SIZE)
        return false;

    uint8_t btn = 0;

    if(m_buf[0].keycode == LEFT_CLICK) btn = KC_MS_BTN1;
    if(m_buf[0].keycode == RIGHT_CLICK) btn = KC_MS_BTN2;

    if(btn && m_buf[0].pressed){
        // ダブル
        if(m_buf_len >= 3 &&
            !m_buf[1].pressed && m_buf[2].pressed && 
            m_buf[2].keycode == m_buf[0].keycode){

            tap_code(btn);
            tap_code(btn);
            clear_m_buf();
            return true;
        }

        // ドラッグ
        if(m_buf_len == 1){
            register_code(btn);
            m_dragging = true;
            clear_m_buf();
            return true;
        }

        // シングル
        if(m_buf_len >= 2 && !m_buf[1].pressed){
            tap_code(btn);
            clear_m_buf();
            return true;
        }

        // 通常出力
        for(uint8_t i=0; i<m_buf_len; i++){
            if (m_buf[i].pressed) {
                tap_code(m_buf[i].keycode);
            }
        }
    }
    
    clear_m_buf();
    return false;
    
}

// =============================
// 外部呼び出し用関数
// =============================

// M_MODE の処理を行う関数
void mouse_mode_toggle(void) {
    m_mode = !m_mode;
}

// M_CFG_TERM_DEC / INC の処理を行う関数
// 引数に true を渡すと増加(INC)、false を渡すと減少(DEC)します
// 設定が変更されたら true、変更されなかったら false を返します
void mouse_mode_change_term(bool is_increment) {
    if (is_increment) {
        if (M_TERM < 200) {
            M_TERM = M_TERM + 10;
        }
    } else {
        if (M_TERM > 10) {
            M_TERM = M_TERM - 10;
        }
    }
}

// =============================
// getter
// =============================
bool get_m_mode(void){
    return m_mode && latast_move_mouse;
}

uint8_t mouse_mode_get_term(void){
    return M_TERM;
}

void mouse_mode_set_term(uint8_t input){
    M_TERM = input;
}


