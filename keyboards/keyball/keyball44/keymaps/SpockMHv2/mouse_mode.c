#include "mouse_mode.h"
#include "eeconfig.h"
#include "rgblight.h"


#define M_CFG_BASE QK_USER_3
#define M_BUF_SIZE 3

#define LEFT_CLICK  KC_J
#define RIGHT_CLICK KC_L
#define shift_key   0x2207
#define ctrl_key    0x2150
#define layer4_key  0x440E

#define M_MODE             (M_CFG_BASE + 0)
#define M_CFG_TERM_DEC     (M_CFG_BASE + 1)
#define M_CFG_TERM_INC     (M_CFG_BASE + 2)

// =============================
// EEPROM
// =============================

typedef union {
    uint32_t raw;
    struct {
        uint8_t  m_term;
        uint8_t  rgblight_hue;
        uint8_t  Reserved1;
        uint8_t  tetris_hi_score;
        uint8_t  Reserved2;
    };
} user_config_t;

static user_config_t user_config = {
    .m_term = 60,
    .rgblight_hue = 147,
    .Reserved1 = 0,
    .tetris_hi_score = 1,
    .Reserved2 = 0
};

#define M_TERM       user_config.m_term
#define HUE          user_config.rgblight_hue

// =============================
// Jバッファ
// =============================

typedef struct {
    uint8_t keycode;
    bool pressed;
} key_event_t;

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

static void save_user_config(void){
    eeconfig_update_user(user_config.raw);
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
// 初期化
// =============================
void mouse_mode_init(void){
    user_config.raw = eeconfig_read_user();
    rgblight_sethue(HUE);
}

// =============================
// process_record_user 側
// =============================

bool inline mouse_mode_process(uint16_t keycode, keyrecord_t *record){
    
    // =============================
    // 設定キー処理（最優先）
    // =============================
    if(record->event.pressed){
        bool setting_press = false;
        switch(keycode){
            case QK_USER_1:
                HUE += 3;
                rgblight_sethue(HUE);
                setting_press = true;
                break;
            
            case M_MODE:
                m_mode = !m_mode;
                break;

            case M_CFG_TERM_DEC:
                if(M_TERM > 10){
                    M_TERM -= 10;
                    setting_press = true;
                }
                break;

            case M_CFG_TERM_INC:
                if(M_TERM < 250){
                    M_TERM += 10;
                    setting_press = true;
                }
                break;
        }
        if (setting_press) {
            save_user_config();
            return false;
        }
    }


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
// getter
// =============================
bool get_m_mode(void){
    return m_mode && latast_move_mouse;
}

uint8_t mouse_mode_get_term(void){
    return M_TERM / 10;
}

uint8_t get_hue(void){
    return HUE;
}


#ifndef BALL

uint8_t get_hi_score(void){
    return user_config.tetris_hi_score;
}
void set_hi_score(uint8_t score){
    user_config.tetris_hi_score = score;
    save_user_config();
    return;
}

#endif
