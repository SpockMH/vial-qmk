bool oled_task_user_func(void);
void oled_update(void);

// 右手OLED状態同期用のRPCハンドラを登録する。
// keymap.c の keyboard_post_init_user() から無条件で呼ぶこと
// (左右どちらがマスターになっても受信側として機能する必要があるため)。
void oled_status_sync_register(void);

// マスター側で状態変化を検知し、必要ならRPC送信する。
// keymap.c の housekeeping_task_user() から無条件で呼ぶこと。
// OLEDハードウェアの有無に関係なく左右どちらのMCUでも確実に実行される
// housekeeping_task_user() 経由にすることで、OLEDを持たない側がマスターに
// なった場合でも同期が止まらないようにしている。
void oled_status_sync_task(void);