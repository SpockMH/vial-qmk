# display/ — OLED表示

マスター側(通常右手)のOLEDに、レイヤー状態・各種設定値・ペット風アニメーション(Luna)を表示する。

## ファイル一覧

| ファイル | 役割 | ビルド対象か |
|---|---|---|
| `oled_user.c/h` | OLED描画のメインロジック(状態画面・設定画面・Lunaアニメーション) | ✅ (`rules.mk`に登録) |
| `oled_7seg.c/h` | 7セグメントLED風の数字/英字を任意座標に描画する汎用ヘルパー | ✅ (`rules.mk`に登録) |
| `font.c` | OLED用のカスタムフォント(容量削減版、`config.h`の`OLED_FONT_H`で指定) | ✅ (フォントとしてinclude) |
| `tetris.c` | OLED上で動くテトリス風ミニゲーム | ❌ 現在不使用 |
  
## 画面構成(oled_user.c)

マスター側は`layer_state_is(2)`(Fnレイヤー相当)の有無で2つの画面を切り替える:

1. **通常画面(`print_lock_key_status`)**: 現在のレイヤー番号(`oled_7seg`で描画)・JP/US切替状態・CapsLock/CapsWord/スプラッシュ/矢印モードのON-OFF表示・Lunaアニメーション・M-MODEのON-OFF表示
2. **設定画面(`setting_status`)**: CPI値・M-MODEの判定時間(JT)・マウス速度スムージングの各パラメータ(上限/下限閾値・最小倍率・直近の移動量合計)

スレーブ側は`oledkit.c`(`lib/oledkit/`)側のデフォルト実装によりロゴのみ表示する。

## oled_7seg（7セグメント風文字描画）

任意の座標(x, y)に32x32pxの7セグメントLED風文字を描画する汎用ヘルパーモジュール。`oled_user.c`から独立しており、他の画面表示にも流用できる。

- `oled_write_7seg_char(x, y, c)`: 文字1つを指定座標に描画する。対応文字は`0`-`9`, `A`, `b`, `C`, `d`, `E`, `F`, `-`, 空白。
- `oled_write_7seg_num(x, y, num)`: 0〜9の数値を描画する薄いラッパー(`num`が9を超える場合は9に丸められる)。

### 実装方式

7セグメント(A〜G)それぞれに対応する32x32pxのビットマップ(`seg7_A`〜`seg7_G`、各128byte、`PROGMEM`上)を持ち、表示したい文字に応じたセグメントの組み合わせ(`get_seg_mask()`のビットマスク)だけをOR合成してから、4ページ分に分けて`oled_write_raw()`で描画する。

### 旧実装からの置き換え

以前は`oled_user.c`内に個別の固定ビットマップ(`img_num0`〜`img_num4`、レイヤー番号0〜4専用)を持っていたが、`oled_7seg`導入によりこれらは削除され、汎用的な`oled_write_7seg_num()`呼び出しに統一された。これにより「0〜4」に限らず任意の数字を表示できるようになっている。

## Lunaアニメーション

WPM(Words Per Minute、直近のタイピング速度)とレイヤー/CapsLock状態に応じて、犬のキャラクター(Luna)の座り/歩き/走り/吠え/忍び足の5パターンを2フレームでアニメーションさせる。フレームデータはすべて`PROGMEM`上のビットマップ(`sit`/`walk`/`run`/`bark`/`sneak`)。

## フォント(font.c)

`keyboards/keyball/lib/logofont/logofont.c`をベースに、容量削減のため記号・小文字アルファベット・Keyballロゴ部分を削除したカスタムフォント。`config.h`で`OLED_FONT_START=48`, `OLED_FONT_END=90`が指定されており、数字と大文字アルファベット中心の範囲のみを使用する。

## 依存関係

```
oled_user.c
  └─ oled_7seg.h (oled_write_7seg_num)  ← レイヤー番号の描画に使用
```

`oled_7seg.c`自体は`quantum.h`/`oled_driver.h`のみに依存し、他のfeatureモジュールへの依存はない。
