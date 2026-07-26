# features/ — Keyball44 vialキーマップ独自機能

QMK標準機能だけでは実現できない、このキーマップ独自の拡張機能をまとめたディレクトリ。機能領域ごとにサブディレクトリへ分割している。

## vial/ ルート直下のファイルとの関係

`features/`配下のモジュール群は、`vial/`直下にある`keymap.c`(エントリポイント。レイヤー定義`keymaps[][]`と、各featureモジュールを呼び出すハンドラ`process_record_user`/`pointing_device_task_user`/`layer_state_set_user`/`keyboard_post_init_user`などを持つ)から利用される。

このキーマップ独自のキーコード定義(`enum custom_keycodes`)とそのディスパッチ関数は[`features/key_control/custom_keycodes.c/h`](key_control/README.md)にある(以前は`vial/`直下に配置していたが、「キーコード変換」を担う`input/`と役割が近いため統合し、ディレクトリ名も`key_control/`に変更した)。

## ディレクトリ構成

| ディレクトリ | 概要 | 詳細 |
|---|---|---|
| [`pointing/`](pointing/README.md) | トラックボール(PMW3360/AZ1UBALL)のモーション処理全般。マウス移動・矢印キー変換・ジェスチャー・速度調整 | [README](pointing/README.md) |
| [`lighting/`](lighting/README.md) | RGBライティング(WS2812)制御。エフェクトモード管理、左右同期、スプラッシュ演出、ライティング座標追跡 | [README](lighting/README.md) |
| [`display/`](display/README.md) | OLED表示。レイヤー/設定状態の表示、7セグメント風文字描画、Lunaアニメーション、カスタムフォント | [README](display/README.md) |
| [`key_control/`](key_control/README.md) | キーコード定義・変換・ディスパッチ系。独自キーコード定義(`custom_keycodes`)、JIS→US変換、単語/行単位のテキスト選択拡張、その他の小さな独立処理 | [README](key_control/README.md) |
| [`config/`](config/README.md) | EEPROM永続化。ユーザー設定値の読み書き | [README](config/README.md) |

## 全体依存関係の概要

```
keymap.c (エントリポイント: process_record_user / pointing_device_task_user / layer_state_set_user 等)
  │
  ├─ key_control/ (custom_keycodes, jis2us, select_extend, utils)
  │     └─ custom_keycodes.c が pointing/・lighting/・config/ の一部関数を横断的に呼ぶ(下記参照)
  │
  ├─ pointing/   (mouse_mode, mouse_speed_smoothing, mouse2key,
  │               az1uball_gesture, virtual_key)
  │     └─ az1uball_gesture.c は lighting/lighting_tracking.h に依存
  ├─ lighting/   (rgblight_user, lighting_tracking) ← pointing/ と display/ から呼ばれる
  │     └─ lighting_tracking.c は pointing/mouse_mode.h に依存(clear_m_buf)
  ├─ display/    (oled_user, font)                ← pointing/・key_control/・lighting/ の状態を読み取って表示
  └─ config/     (eeconfig_user)                  ← pointing/・lighting/ の設定値を読み書き
```

依存の向きは概ね「`config/`・`display/`が他モジュールの状態を参照する」「`pointing/`が`lighting/`を駆動する」「`key_control/`が他カテゴリの操作関数を横断的に呼ぶ」形になっている。

## 命名・配置のルール

- 新しい機能を追加する場合は、上記5分類のいずれかに該当するディレクトリへ配置する。どれにも当てはまらない場合は新しいサブディレクトリを作成し、このREADMEに追記すること。
- クロスディレクトリでヘッダをincludeする場合は、`features/<サブディレクトリ>/<ファイル名>.h`の形式でフルパス指定する(同一ディレクトリ内であれば`"ファイル名.h"`のみでも可)。
- 各サブディレクトリのREADME.mdには「ファイル一覧」「機能概要」「依存関係」を最低限記載する。既知の不具合・未使用コードなどがあれば「既知の課題」として明記する。

