# ---- Vial / VIA ----
VIA_ENABLE = yes
VIAL_ENABLE = yes
VIAL_INSECURE = yes

# ---- 入力系基本機能 ----
QMK_SETTINGS = yes
TAP_DANCE_ENABLE = yes
COMBO_ENABLE = yes
KEY_OVERRIDES_ENABLE = yes
EXTRAKEY_ENABLE = yes
MAGIC_ENABLE = yes
GRAVE_ESC_ENABLE = yes
MOUSEKEY_ENABLE = yes
NKRO_ENABLE = yes
CAPS_WORD_ENABLE = yes
DYNAMIC_MACRO_ENABLE = yes
WPM_ENABLE = yes

# ---- RGB / OLED / ポインティングデバイス ----
WS2812_DRIVER_REQUIRED = yes
WS2812_DRIVER = vendor
RGBLIGHT_ENABLE = no
VIALRGB_ENABLE = no
OLED_ENABLE = yes
POINTING_DEVICE_ENABLE = yes


# ---- その他 ----
RAW_ENABLE = yes
LTO_ENABLE = no
MOUSE_SHARED_EP = no
BOOTMAGIC_ENABLE = no
DEBOUNCE_TYPE = sym_eager_pk

# ---- ビルド対象の追加ソース ----
SRC += quantum/color.c
SRC += features/rgblight_user.c
SRC += features/mouse_mode.c
SRC += features/eeconfig_user.c
SRC += features/jis2us.c
SRC += features/oled_user.c
SRC += features/arrow_layer.c
SRC += features/gesture_layer.c
SRC += features/select_extend.c
SRC += features/mouse_speed_smoothing.c

SRC += az1uball/az1uball.c
SRC += az1uball/az1uball_dpad.c