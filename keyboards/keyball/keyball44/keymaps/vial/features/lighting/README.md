# lighting/ — RGBライティング制御

Keyball44の60個のWS2812 LED(左右分割・合計)を制御する独自RGBエフェクトエンジン。QMK標準の`RGBLIGHT_ENABLE`は`rules.mk`で無効化されており(`RGBLIGHT_ENABLE = no`)、代わりにこのモジュールが直接WS2812を制御する。

## ファイル一覧

| ファイル | 役割 |
|---|---|
| `rgblight_user.c/h` | エフェクトモード管理・描画・左右同期のすべてを担う中心モジュール |
| `lighting_tracking.c/h` | マウスの移動量からライティング用のキー座標(row/col)を追跡し、`rgblight_user`のエフェクトを駆動する |

## エフェクトモード

`enum RGBLIGHT_EFFECT_MODE`で定義される4種類:

| モード | 内容 |
|---|---|
| `RGBLIGHT_MODE_OFF` | 消灯 |
| `RGBLIGHT_MODE_ICEWAVE` | 彩度が波打つように変化するアイドル演出(layer1で使用) |
| `RGBLIGHT_MODE_STATIC` | 単色固定点灯(layer2で使用) |
| `RGBLIGHT_MODE_MOUSEMOVE` | トラックボール操作に反応してキー周辺が光る(通常時に使用) |
| `RGBLIGHT_MODE_SCROLLMOVE` | スクロール操作時、行全体が光る(スクロールレイヤーで使用) |

モード切替は`keymap.c`の`layer_state_set_user()`から`rgblight_mode()`を呼ぶことで行われる。

## 左右同期の仕組み

マスター(通常は右手)からスレーブへ`transaction_rpc_send(USER_SYNC_LIGHTING, ...)`で`rgblight_simple_config_t`(enable/mode/hue/sat/val)を送信し、スレーブ側で`rgblight_update_sync()`が呼ばれて反映される。変化があった項目のみ即座にLEDへ反映する。

## スプラッシュ(波紋)エフェクト

`rgblight_value()`にトリガーフラグを渡すと、押下されたキー位置を中心に波紋が広がるエフェクトが発生する。最大`SPLASH_MAX_COUNT`(10個)まで同時発生でき、リングバッファ方式で古いものから上書きされる。距離計算(`keyball_distance`)は左右分割キーボードの物理配置を考慮した独自メトリクスを使用。

## lighting_tracking（マウス移動→ライティング座標追跡）

`RGBLIGHT_MODE_MOUSEMOVE`/`RGBLIGHT_MODE_SCROLLMOVE`でどのキー位置を光らせるかを決めるための、独立したモジュール。トラックボールのモーション(`pointing_device_task_user`から渡される`mouse_report`)を受け取る点では`pointing/`の処理に近いが、目的が「ライティング座標の算出」であるため`lighting/`に配置している。

- 右手基準の座標系(row 4〜6)でキー座標(`sync_data.key_row/key_col`)を管理する
- マウスのx/y/vをテンションとして蓄積し、閾値(`TENSION_THRESHOLD`)を超えたら座標を1マス移動させる
- 座標が変化したら`rgblight_value()`(本ディレクトリの`rgblight_user.c`)を呼びつつ、`transaction_rpc_send(USER_SYNC_KEY_COUNTER, ...)`でスレーブへ座標を同期する
- 左手側の座標が残っている場合は`row' = row + 4, col' = 5 - col`で自動的に右手基準へ正規化する(`LIGHTING_ROW_OFFSET`/`LIGHTING_COL_MAX`)

`keymap.c`からは`lighting_tracking_init()`(post_init時)、`lighting_tracking_update()`(マウス移動時)、`lighting_tracking_set_position()`/`lighting_tracking_trigger()`(キー押下・クリック・レイヤー切替時)、`lighting_tracking_rpc_handler()`(スレーブ側RPCハンドラ登録用)が呼ばれる。

## 依存関係

`rgblight_user.c`は他のfeatureモジュールに依存しない。`lighting_tracking.c`は同ディレクトリの`rgblight_user.h`(`rgblight_value`)と、`features/pointing/mouse_mode.h`(`clear_m_buf`)に依存する。`keymap.c`からは`rgblight_mode()`/`rgblight_init()`/`rgblight_task()`/`rgblight_enable()`/`rgblight_disable()`(rgblight_user側)、および前述のlighting_tracking関数群が呼ばれる。

## アイドル消灯

`keymap.c`側の`housekeeping_task_user()`で、`RGB_IDLE_TIMEOUT_MS`(30秒)操作がなければ自動的に`rgblight_disable()`される。次の操作で`wake_rgb()`により自動復帰する。
