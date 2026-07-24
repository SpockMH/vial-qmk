#pragma once

#include "quantum.h"

void jis2us_toggle(void);
bool get_jis2us(void);
bool jis2us_process(uint16_t keycode, keyrecord_t *record);