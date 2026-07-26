# config/ — EEPROM永続化

キーボード再起動後も保持したいユーザー設定値を、EEPROMのユーザーデータブロックに読み書きする。

## ファイル一覧

| ファイル | 役割 |
|---|---|
| `eeconfig_user.c/h` | ユーザー設定構造体の定義と、EEPROMへの読み書き処理 |

## 保存される設定値

`user_config_t`構造体(`EECONFIG_USER_DATA_SIZE=16`byteに合わせて設計):

| フィールド | サイズ | 内容 |
|---|---|---|
| `speed_lower_threshold` | 2byte | マウス速度スムージングの下限閾値 |
| `speed_upper_threshold` | 2byte | マウス速度スムージングの上限閾値 |
| `speed_min_scale_pct` | 1byte | マウス速度スムージングの最小倍率(%) |
| `m_term` | 1byte | M-MODE(mouse_mode)のクリック判定時間 |
| `hue` | 1byte | RGBライティングの色相(Hue) |
| `reserved` | 9byte | 将来の拡張用(現在未使用) |

フィールドはサイズの大きい順に並べ、構造体アライメントによる余計なパディングを防いでいる。

## 読み書きのタイミング

- **読み込み**: `keyboard_post_init_user()`から呼ばれる`user_config_init()`で、初回起動時はデフォルト値を書き込み、2回目以降はEEPROMから読み込んで各モジュール(`mouse_mode`/`rgblight_user`/`mouse_speed_smoothing`)に設定値を反映する。
- **書き込み**: `KBC_SAVE`キー押下時に呼ばれる`save_user_config()`で、各モジュールから現在値を取得してEEPROMに書き込む。**自動保存ではないため、設定変更後は明示的に`KBC_SAVE`を押す必要がある**。

## 依存関係

```
eeconfig_user.c
  ├─ features/pointing/mouse_mode.h      (mouse_mode_get_term / _set_term)
  ├─ features/lighting/rgblight_user.h   (get_hue / set_hue)
  └─ features/pointing/mouse_speed_smoothing.h
        (mouse_speed_smoothing_get_config / _set_config)
```

`keymap.c`からは`user_config_init()`(post_init時)と`save_user_config()`(`KBC_SAVE`キー押下時)のみが呼ばれる。
