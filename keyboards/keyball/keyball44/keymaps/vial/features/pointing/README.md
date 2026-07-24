# pointing/ — トラックボール入力処理

トラックボール(PMW3360/AZ1UBALL)のモーションデータを、マウス移動・スクロール・矢印キー・ジェスチャー・ライティング座標などに変換する処理群。

## ファイル一覧

| ファイル | 役割 |
|---|---|
| `mouse_mode.c/h` | 左クリック(`J`キー)を一定時間バッファし、シングル/ダブルクリック・ドラッグを判定する「M-MODE」機能 |
| `mouse_speed_smoothing.c/h` | 直近の移動量から速度を算出し、低速時のカーソル移動量を可変倍率でスムージングする |
| `arrow_layer.c/h` | layer1/2でのトラックボール移動を矢印キー/BSDEL・DELキーに変換する(テンション式) |
| `az1uball_gesture.c/h` | AZ1UBALL(左手側トラックボール)の移動をD-padジェスチャー(上下左右+クリック)に変換する |
| `virtual_key.c/h` | 仮想マトリクス位置(row, col)に割り当てられたキーマップ上のキーコードを、`process_record_user`を経由せずに直接発火する仕組み |

> **注**: ライティング用キー座標の追跡(`lighting_tracking.c/h`)は「トラックボール入力」よりも「ライティング制御」の一部としての性質が強いため、[`features/lighting/`](../lighting/README.md)に配置しています。トラックボールのモーションを受け取って処理する点ではpointing/と関連が深いので、必要ならそちらも参照してください。

## 依存関係

```
keymap.c
  ├─ pointing_device_task_user() で各モジュールを呼び出し
  │
  ├─ mouse_mode.c        (単独。keymap.cのprocess_record_user経由でキーイベントを受け取る)
  ├─ mouse_speed_smoothing.c
  │     └─ features/config/eeconfig_user.h (閾値設定の永続化用。※現状未使用の可能性あり、下記「既知の課題」参照)
  ├─ arrow_layer.c        (単独)
  ├─ az1uball_gesture.c
  │     └─ virtual_key.c  (ジェスチャー発火時にキーを叩く)
  └─ (lighting_tracking.c は features/lighting/ に移動済み。pointing_device_task_user から
      lighting_tracking_update() が呼ばれるが、実体はlighting/配下にある)
```

## 各機能の概要

### mouse_mode（M-MODE）
`J`キー(左クリック相当)の押下パターンを`M_TERM`ms監視し、シングルクリック/ダブルクリック/ドラッグを判別してマウスボタンイベントに変換する。`MMD_TOG`でON/OFF、`MMT_INC`/`MMT_DEC`で判定時間を調整できる。

### mouse_speed_smoothing
直近10サンプルの移動量合計(`speed_buf`)をもとに、台形カーブで速度倍率(`min_scale_pct`〜100%)を算出し、低速移動を滑らかにする。`SPD_THL_*`/`SPD_THU_*`/`SPD_SCL_*`キーで下限閾値・上限閾値・最小倍率をリアルタイム調整でき、`KBC_SAVE`でEEPROMに保存される。

### arrow_layer
layer1(左右のみ: BSPC/DEL)・layer2(上下左右: 矢印キー)で、トラックボールの動きを「テンション」として蓄積し、一定量を超えたら該当方向のキーを1回タップする。垂直方向に発火した直後は一定時間水平方向の発火を抑制し、斜め移動時の誤爆を防ぐ。

### az1uball_gesture
左手側のAZ1UBALLの動きを、状態遷移(IDLE→MOVING→LOCKED)で管理し、一定時間(`GESTURE_FIRE_DELAY_MS`)後にテンションの向きに応じたD-padキーを`virtual_key.c`経由で発火する。クリックは即座に別キーとして発火する。

### virtual_key
Vialのキーマップ編集画面で自由に割り当てられる「仮想マトリクス位置」(未使用の物理キー相当)に対して、現在のレイヤーで割り当てられているキーコードを検索し、`tap_code16()`または`process_user_virtual_keycode()`経由で発火する。`az1uball_gesture.c`から利用される。

## 既知の課題

- `mouse_speed_smoothing.c`が`eeconfig_user.h`をincludeしているが、ファイル内で直接使用している形跡がない(実際の永続化は`features/config/eeconfig_user.c`側から`mouse_speed_smoothing_get_config()`/`_set_config()`を呼ぶ形で行われている)。不要なincludeの可能性があるため要確認。
