# key_control/ — キーコード定義・変換・ディスパッチ

このキーマップ独自のキーコード定義から、物理キー入力の変換・拡張、独自キーコードの処理ディスパッチまでをまとめたディレクトリ。トラックボールとは無関係の、純粋なキー入力処理群。

## ファイル一覧

| ファイル | 役割 |
|---|---|
| `custom_keycodes.c/h` | このキーマップ独自のキーコード定義(`enum custom_keycodes`)と、それらを処理するディスパッチ関数 |
| `jis2us.c/h` | JIS配列のキーコードをUS配列相当のキーコードへ変換する |
| `select_extend.c/h` | 単語/行単位でのテキスト選択を「押すたびに選択範囲が拡張される」形式で行う |
| `utils.c/h` | `process_record_user`から呼ばれる小さな独立処理(CPI一時倍率、IMEかな/英数追従)をまとめたユーティリティ |

> **旧ディレクトリ名**: `features/input/`。`custom_keycodes.c/h`をこのディレクトリに統合したタイミングで、「キーコード変換」だけでなく「キーコード定義・ディスパッチ」も含む名称として`key_control/`に変更した。以前は`custom_keycodes.c/h`は`vial/`直下(`keymap.c`と同階層)に配置されていた。

## custom_keycodes

このキーマップ独自のキーコードを`enum custom_keycodes`で定義し(`KEYBALL_SAFE_RANGE`を起点)、それらを処理する2つのディスパッチ関数を提供する。

- `process_user_keycode(keycode, record)`: 押下時のみ処理する系統(`JPUS_TOG`/`MMD_TOG`/`MMT_INC`/`MMT_DEC`/`LIGHT_*`/`HUE_UP`/`SPL_TOG`/`KBC_SAVE`)。`keymap.c`の`process_record_user`から呼ばれる。
- `process_user_virtual_keycode(keycode)`: `SELWORD_*`/`SELLINE_*`(選択拡張)と`QK_TOGGLE_LAYER`系の処理。`features/pointing/virtual_key.c`の`fire_virtual_key()`経由(0x7E00以降のキーコード)、および`keymap.c`の`process_record_user`から直接の両方から呼ばれる。

また、`select_extend.h`/`mouse_speed_smoothing.h`側の「weakなextern変数」(`SELECT_WORD_LEFT_KEYCODE`など)に実際のキーコード値を紐付ける代入もここで行っている。

スプラッシュ演出の強制ON/OFF(`SPL_TOG`)は、`features/lighting/rgblight_user.c`の`rgblight_toggle_splash_mode()`を呼ぶことで切り替える(状態自体は`rgblight_user.c`内に閉じており、`custom_keycodes.c`側では保持しない)。

## jis2us

`JPUS_TOG`キーでON/OFFを切り替える。ONの場合、記号キー(`(` `)` `@` `[` `]` `{` `}` `-` `=` など)をJIS配列の入力からUS配列相当の出力に変換して送出する。Shift併用時の変換にも対応。

OFF時(デフォルトのJIS入力)でも、Shift/CapsWord併用時の `-` を `_` に変換する専用処理が入っている(US配列に合わせるため)。

## select_extend

矢印キー4つ相当のキー(`SELWORD_LEFT`/`SELWORD_RIGHT`/`SELLINE_UP`/`SELLINE_DOWN`)を、押すたびに選択範囲を拡張していく方式で実装。

- 初回押下: カーソル位置から選択を開始(単語/行の切れ目まで一旦移動してから選択)
- 2回目以降: 現在の選択を維持したまま、さらに1単位分拡張
- 選択操作以外のキーが押されると自動的に選択状態がリセットされる(`process_record_user`から呼ばれる`process_select_extend`が判定)

キーコードは同ディレクトリの`custom_keycodes.c`側でグローバル変数(`SELECT_WORD_LEFT_KEYCODE`など)に代入することで紐付けられている(weak変数パターン)。

## utils

`process_record_user`から呼ばれる、他のどのカテゴリにも属さない小さな独立処理をまとめたモジュール。

- `process_cpi_x3()`: `CPI_x3`キーを押している間だけCPIを3倍にし、離すと元に戻す
- `process_kana_ime()`: IME(かな/英数)の状態を追従し、変換候補ウィンドウ操作後や`KC_LEFT_BRACKET`押下時に自動でIME ON(かな)へ戻す

`process_cpi_x3()`は同ディレクトリの`custom_keycodes.h`の`CPI_x3`キーコードに依存する。

## 依存関係

```
custom_keycodes.c
  ├─ jis2us.h / select_extend.h          (同ディレクトリ)
  ├─ features/pointing/mouse_mode.h      (mouse_mode_toggle / _change_term)
  ├─ features/lighting/rgblight_user.h   (rgblight_increase_val / _decrease_val / get_hue / set_hue)
  └─ features/config/eeconfig_user.h     (save_user_config)

utils.c
  ├─ custom_keycodes.h (同ディレクトリ。CPI_x3キーコード)
  └─ lib/keyball/keyball.h (keyball_get_cpi / _set_cpi)

jis2us.c / select_extend.c
  └─ 他のfeatureモジュールに依存しない独立した機能
```

`keymap.c`からは`process_user_keycode()`/`process_user_virtual_keycode()`(custom_keycodes側)、`process_cpi_x3()`/`process_kana_ime()`(utils側)、`jis2us_toggle()`/`jis2us_process()`(jis2us側)、`process_select_extend()`(select_extend側)が`process_record_user()`から呼ばれる。
