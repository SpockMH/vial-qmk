# RP2040（Raspberry Pi Silicon）用の設定
MCU = RP2040
BOOTLOADER = rp2040

# Link Time Optimization - RP2040 split RPC通信と競合するため無効
# LTO_ENABLE = yes

# Build Options
BOOTMAGIC_ENABLE = no
EXTRAKEY_ENABLE = no
CONSOLE_ENABLE = no
COMMAND_ENABLE = no
NKRO_ENABLE = no
BACKLIGHT_ENABLE = no
AUDIO_ENABLE = no

SERIAL_DRIVER = vendor
# WS2812_DRIVER = vendor   ← 削除（デフォルトに任せる）

# Keyball44 is split keyboard.
# SPLIT_KEYBOARD = yes   ← 削除（keyboard.jsonの "split": {"enabled": true} に任せる）

# Optical sensor driver for trackball.
POINTING_DEVICE_ENABLE = yes
POINTING_DEVICE_DRIVER = custom
SRC += drivers/pmw3360/pmw3360.c
SRC += drivers/az1uball/az1uball.c
QUANTUM_LIB_SRC += spi_master.c # Optical sensor use SPI to communicate


MOUSEKEY_ENABLE = no

RGBLIGHT_ENABLE = no
RGB_MATRIX_ENABLE = no
RGB_MATRIX_DRIVER = ws2812

SLEEP_LED_ENABLE = no

OLED_ENABLE = no
SRC += lib/oledkit/oledkit.c

SRC += lib/keyball/keyball.c

SPACE_CADET_ENABLE = no
GRAVE_ESC_ENABLE = no
MAGIC_ENABLE = no