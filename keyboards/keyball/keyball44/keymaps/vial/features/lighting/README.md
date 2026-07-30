# lighting/ — RGBライティング制御

Keyball44の60個のWS2812 LED(左右分割・合計)を制御する独自RGBエフェクトエンジン。QMK標準の`RGBLIGHT_ENABLE`は`rules.mk`で無効化されており(`RGBLIGHT_ENABLE = no`)、代わりにこのモジュールが直接WS2812を制御する。

## ファイル一覧

| ファイル | 役割 |
|---|---|
| `rgblight_user.c/h` | エフェクトモード管理・描画・左右同期のすべてを担う中心モジュール |
| `lighting_tracking.c/h` | マウスの移動量からライティング用のキー座標(row/col)を追跡し、`rgblight_user`のエフェクトを駆動する |

## エフェクトモード

`enum RGBLIGHT_EFFECT_MODE`で定義される6種類:

| モード | 内容 |
|---|---|
| `RGBLIGHT_MODE_OFF` | 消灯 |
| `RGBLIGHT_MODE_ICEWAVE` | 彩度が波打つように変化するアイドル演出(layer1で使用) |
| `RGBLIGHT_MODE_STATIC` | 単色固定点灯(layer2で使用) |
| `RGBLIGHT_MODE_MOUSEMOVE` | トラックボール操作に反応してキー周辺が光る(通常時に使用)。花火・スプラッシュエフェクトもこのモード上に重ねて描画される |
| `RGBLIGHT_MODE_SCROLLMOVE` | スクロール操作時、行全体が光る(スクロールレイヤーで使用) |
| `RGBLIGHT_MODE_SWIRL` | 列ごとに色相をずらした渦巻き状の演出 |
| `RGBLIGHT_MODE_CROSS` | 行列の位置に応じて市松模様に2色を交互点灯させる演出 |

モード切替は`keymap.c`の`layer_state_set_user()`から`rgblight_mode()`を呼ぶことで行われる。

## 左右同期の仕組み

マスター(通常は右手)からスレーブへ`transaction_rpc_send(USER_SYNC_LIGHTING, ...)`で`rgblight_simple_config_t`(enable/mode/hue/sat/val)を送信する。スレーブ側では、本ディレクトリの`rgblight_user.c`にある`rgblight_sync_rpc_handler()`がRPCを受け取り、`rgblight_update_sync()`に委譲する。変化があった項目のみ即座にLEDへ反映する。

`USER_SYNC_KEY_COUNTER`(`lighting_tracking_rpc_handler`)と`USER_SYNC_LIGHTING`(`rgblight_sync_rpc_handler`)のRPC登録は`lighting_register_rpc_handlers()`にまとめられており、さらに`rgblight_init()`の内部から呼ばれる。そのため`keymap.c`の`keyboard_post_init_user()`は`rgblight_init()`を呼ぶだけでよく、RPC登録を個別に意識する必要はない。

## スプラッシュ(波紋)エフェクト

`rgblight_value()`の`splash_trig`引数に`true`を渡すと、押下されたキー位置を中心に波紋が広がるエフェクトが発生する。最大`SPLASH_MAX_COUNT`(10個)まで同時発生でき、リングバッファ方式で古いものから上書きされる。距離計算(`keyball_distance`)は左右分割キーボードの物理配置を考慮した独自メトリクスを使用。

`splash_trig`は`lighting_tracking_trigger(scr, click)`経由で呼び出し側(M-MODEのクリック確定・AZ1UBALLのクリックなど)が明示的に`click=true`を渡した場合にのみ発生する。

## 花火風スプラッシュ(高速タイピング時)

高速タイピング中(`get_current_wpm() > 40`)は、押されたキーごとに、上記のスプラッシュ(波紋)エフェクトを流用した小さめの「花火風」バーストを発生させる。`splash_trig`によるクリック時のスプラッシュとは別の独立したロジックとして`rgblight_value()`内に実装されており、`splash_trig`が`true`の場合はそちらが優先される(同一フレームで両方が同時に発生することはない)。`update=true`(キー押下・レイヤー切替・M-MODE/AZ1UBALLクリックなど、実際の座標更新イベント)の場合にのみ発生し、`update=false`(`rgblight_effect_mousemove`/`rgblight_effect_scrollmove`からの毎フレームのアニメーション更新)は対象外(対象にすると毎フレーム発生し続けてしまうため)。

以前は同条件で花火専用の別実装(hanabi、塗りつぶし円のバースト)を用意していたが、色合いがスプラッシュほど良くならなかったため廃止し、代わりにスプラッシュの波紋ロジックそのものを流用しつつ「最大到達半径だけを小さくする」方式に変更した。`splash_state_t`に`firework`(花火風かどうか)と`max_radius`(そのスプラッシュ固有の最大到達半径。発火した瞬間の値で以後固定)を追加し、処理ループでは`SPLASH_MAX_RADIUS`の代わりに`splash_pool[s].max_radius`を参照する形に一般化してある。

- **最大到達半径はWPMに応じて可変**(`splash_firework_radius_for_wpm()`、発火した瞬間のWPMで決定し、そのスプラッシュが消えるまで固定):
  - WPM 40〜`SPLASH_FIREWORK_GROW_WPM_LOW`(70): `SPLASH_FIREWORK_MAX_RADIUS`固定の小さい花火
  - WPM `SPLASH_FIREWORK_GROW_WPM_LOW`(70)〜`SPLASH_FIREWORK_GROW_WPM_HIGH`(100): `SPLASH_FIREWORK_MAX_RADIUS`→`SPLASH_MAX_RADIUS`へ線形に拡大
  - WPM `SPLASH_FIREWORK_GROW_WPM_HIGH`(100)以上: `SPLASH_MAX_RADIUS`固定(通常のクリック時スプラッシュと同じ大きさ)
- 波紋の色シフト・減衰の形自体(`WAVE_SOLID`/`WAVE_FADE`に相当する`SPLASH_FIREWORK_WAVE_SOLID`/`SPLASH_FIREWORK_WAVE_FADE`)は通常スプラッシュと同じ値にしてあるため、見た目のクオリティ(色合い)はそのままに、範囲の大きさだけがWPMに応じて変化する

## lighting_tracking（マウス移動→ライティング座標追跡）

`RGBLIGHT_MODE_MOUSEMOVE`/`RGBLIGHT_MODE_SCROLLMOVE`でどのキー位置を光らせるかを決めるための、独立したモジュール。トラックボールのモーション(`pointing_device_task_user`から渡される`mouse_report`)を受け取る点では`pointing/`の処理に近いが、目的が「ライティング座標の算出」であるため`lighting/`に配置している。

- 右手基準の座標系(row 4〜6)でキー座標(`sync_data.key_row/key_col`)を管理する
- マウスのx/y/vをテンションとして蓄積し、閾値(`TENSION_THRESHOLD`)を超えたら座標を1マス移動させる
- 座標が変化したら`rgblight_value()`(本ディレクトリの`rgblight_user.c`)を呼びつつ、`transaction_rpc_send(USER_SYNC_KEY_COUNTER, ...)`でスレーブへ座標を同期する
- 左手側の座標が残っている場合は`row' = row + 4, col' = 5 - col`で自動的に右手基準へ正規化する(`LIGHTING_ROW_OFFSET`/`LIGHTING_COL_MAX`)

`keymap.c`からは`lighting_tracking_init()`(post_init時)、`lighting_tracking_update()`(マウス移動時)、`lighting_tracking_set_position()`/`lighting_tracking_trigger()`(キー押下・クリック・レイヤー切替時)、`lighting_tracking_rpc_handler()`(スレーブ側RPCハンドラ登録用)が呼ばれる。

## 依存関係

`rgblight_user.c`は`rgblight_init()`内部で`lighting_register_rpc_handlers()`を呼ぶために同ディレクトリの`lighting_tracking.h`(`lighting_tracking_rpc_handler`)に依存する。`lighting_tracking.c`は同ディレクトリの`rgblight_user.h`(`rgblight_value`)と、`features/pointing/mouse_mode.h`(`clear_m_buf`)に依存する。`keymap.c`からは`rgblight_init()`/`rgblight_wake()`/`rgblight_task()`/`rgblight_mode()`(rgblight_user側)、および前述のlighting_tracking関数群が呼ばれる。

また`features/pointing/az1uball_gesture.c`の`handle_az1uball_input()`が、AZ1UBALLボタン押下時のライティング即時発火のために`lighting_tracking_set_position()`/`lighting_tracking_trigger()`を呼ぶ(pointing側からlighting側への依存)。

## アイドル消灯

`rgblight_task()`の内部で、`RGB_IDLE_TIMEOUT_MS`(30秒)操作がなければ自動的に`rgblight_disable()`される(旧`keymap.c`の`housekeeping_task_user`から移設)。次の操作で`rgblight_wake()`(`process_record_user`から呼ばれる)により自動復帰する。アイドル管理用の状態(`last_rgb_activity`/`rgb_is_idle`)は`rgblight_user.c`内のモジュール private な static変数で、外部から直接参照することはできない。