#include "jis2us.h"
#include "keymap_japanese.h"

static const uint16_t jis2us[][2] = {
    {JP_LPRN, KC_LPRN},
    {JP_RPRN, KC_RPRN},
    {JP_AT,   KC_AT},
    {JP_LBRC, KC_LBRC},
    {0x31,    KC_RBRC},
    {JP_LCBR, KC_LCBR},
    {0x231,   KC_RCBR},
    {JP_MINS, KC_MINS},
    {JP_EQL,  KC_EQL},
    {JP_BSLS, KC_BSLS},
    {JP_SCLN, KC_SCLN},
    {JP_QUOT, KC_QUOT},
    {JP_GRV,  KC_GRV},
    {JP_PLUS, KC_PLUS},
    {JP_COLN, KC_COLN},
    {JP_UNDS, KC_UNDS},
    {JP_PIPE, KC_PIPE},
    {JP_DQUO, KC_DQT},
    {JP_ASTR, KC_ASTR},
    {JP_TILD, KC_TILD},
    {JP_AMPR, KC_AMPR},
    {JP_CIRC, KC_CIRC},
    {JP_YEN, KC_BSLS},
};

static bool is_us_mode = false;

void jis2us_toggle(void){
    is_us_mode = !is_us_mode;
}

bool get_jis2us(void){
    return is_us_mode;
}

bool jis2us_process(uint16_t keycode, keyrecord_t *record){

    if (!is_us_mode) {
        //SHITFまたはCABSWORDがONの時に、- 入力で _　出力(英語キーボードにそろえる処理)
        uint8_t mods = get_mods();
        if ((keycode & 0xFF) == KC_MINUS && record->event.pressed && 
            (is_caps_word_on() || (mods & MOD_MASK_SHIFT))) {
            tap_code16(0x287);
            return false; 
        }
        return true;
    }

    if (!record->event.pressed) return true;

    uint16_t skeycode;
    bool lshifted = keyboard_report->mods & MOD_BIT(KC_LSFT);
    bool rshifted = keyboard_report->mods & MOD_BIT(KC_RSFT);
    bool shifted = lshifted | rshifted;
    
    if (keycode >= 0x2000){
        keycode = keycode & 0xFF;
    }
    if (shifted) {
        skeycode = QK_LSFT | keycode;
    } else {
        skeycode = keycode;
    }

    for (int i = 0; i 
    < sizeof(jis2us) / sizeof(jis2us[0]); i++) {
        if (jis2us[i][0] == skeycode) {
            unregister_code(KC_LSFT);
            unregister_code(KC_RSFT);
            if ((jis2us[i][1] & QK_LSFT) == QK_LSFT || (jis2us[i][1] & QK_RSFT) == QK_RSFT) {
                register_code(KC_LSFT);
                tap_code(jis2us[i][1]);
                unregister_code(KC_LSFT);
            } else {
                tap_code(jis2us[i][1]);
            }
            if (lshifted) register_code(KC_LSFT);
            if (rshifted) register_code(KC_RSFT);
            return false;
        }
    }

    return true;
}