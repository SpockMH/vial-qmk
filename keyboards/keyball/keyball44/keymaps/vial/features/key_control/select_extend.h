#pragma once
#include "quantum.h"

#ifdef __cplusplus
extern "C" {
#endif

// レイアウト上のキーから使う場合はkeymap.c側で紐付け:
//   uint16_t SELECT_WORD_LEFT_KEYCODE  = SELWORD_LELT;
//   uint16_t SELECT_WORD_RIGHT_KEYCODE = SELWORD_RIGHT;
//   uint16_t SELECT_LINE_UP_KEYCODE    = SELLINE_UP;
//   uint16_t SELECT_LINE_DOWN_KEYCODE  = SELLINE_DOWN;
extern uint16_t SELECT_WORD_LEFT_KEYCODE;
extern uint16_t SELECT_WORD_RIGHT_KEYCODE;
extern uint16_t SELECT_LINE_UP_KEYCODE;
extern uint16_t SELECT_LINE_DOWN_KEYCODE;

// 直接呼び出し用(gesture_layerなどキーコードを経由しない箇所から呼ぶ)
void select_extend_word_left(void);
void select_extend_word_right(void);
void select_extend_line_up(void);
void select_extend_line_down(void);

// 選択モードを強制解除したい場合に呼ぶ
void select_extend_reset(void);

// process_record_user()内で呼ぶ(レイアウト上のキー用)
bool process_select_extend(uint16_t keycode, keyrecord_t* record);

#ifdef __cplusplus
}
#endif