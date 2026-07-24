# display/ — OLED表示

マスター側(通常右手)のOLEDに、レイヤー状態・各種設定値・ペット風アニメーション(Luna)を表示する。

## ファイル一覧

| ファイル | 役割 | ビルド対象か |
|---|---|---|
| `oled_user.c/h` | OLED描画のメインロジック(状態画面・設定画面・Lunaアニメーション) | ✅ (`rules.mk`に登録) |
| `font.c` | OLED用のカスタムフォント(容量削減版、`config.h`の`OLED_FONT_H`で指定) | ✅ (フォントとしてinclude) |
| `tetris.c` | OLED上で動くテトリス風ミニゲーム | ❌ 未登録(下記「既知の課題」参照) |

## 画面構成(oled_user.c)

マスター側は`layer_state_is(2)`(Fnレイヤー相当)の有無で2つの画面を切り替える:

1. **通常画面(`print_lock_key_status`)**: 現在のレイヤー番号・JP/US切替状態・CapsLock/CapsWord/スプラッシュ/矢印モードのON-OFF表示・Lunaアニメーション・M-MODEのON-OFF表示
2. **設定画面(`setting_status`)**: CPI値・M-MODEの判定時間(JT)・マウス速度スムージングの各パラメータ(上限/下限閾値・最小倍率・直近の移動量合計)

スレーブ側は`oledkit.c`(`lib/oledkit/`)側のデフォルト実装によりロゴのみ表示する。

## Lunaアニメーション

WPM(Words Per Minute、直近のタイピング速度)とレイヤー/CapsLock状態に応じて、犬のキャラクター(Luna)の座り/歩き/走り/吠え/忍び足の5パターンを2フレームでアニメーションさせる。フレームデータはすべて`PROGMEM`上のビットマップ(`sit`/`walk`/`run`/`bark`/`sneak`)。

## フォント(font.c)

`keyboards/keyball/lib/logofont/logofont.c`をベースに、容量削減のため記号・小文字アルファベット・Keyballロゴ部分を削除したカスタムフォント。`config.h`で`OLED_FONT_START=48`, `OLED_FONT_END=90`が指定されており、数字と大文字アルファベット中心の範囲のみを使用する。

## 既知の課題

- **`tetris.c`は`rules.mk`のSRCに含まれておらず、現状ビルド対象外**。`tetris_init`/`tetris_update`/`tetris_render`のいずれも`oled_user.c`や`keymap.c`から呼ばれていない(未接続の機能、または開発中/デバッグ用に無効化された状態と思われる)。有効化する場合は`rules.mk`に`SRC += features/display/tetris.c`を追加し、`oled_task_user`または`housekeeping_task_user`から`tetris_update()`/`tetris_render()`を呼ぶ実装が必要。
