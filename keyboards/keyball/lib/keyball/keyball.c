/*
Copyright 2022 @Yowkees
Copyright 2022 MURAOKA Taro (aka KoRoN, @kaoriya)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "quantum.h"
#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

#include "keyball.h"
#include "drivers/pmw3360/pmw3360.h"

#ifdef AZ1UBALL_ENABLE
#    include "drivers/az1uball/az1uball.h"
#endif

#include <string.h>

const uint8_t CPI_DEFAULT    = KEYBALL_CPI_DEFAULT / 100;
const uint8_t CPI_MAX        = pmw3360_MAXCPI + 1;
const uint8_t SCROLL_DIV_MAX = 7;

const uint16_t AML_TIMEOUT_MIN = 100;
const uint16_t AML_TIMEOUT_MAX = 1000;
const uint16_t AML_TIMEOUT_QU  = 50;   // Quantization Unit

static const char BL = '\xB0'; // Blank indicator character
static const char LFSTR_ON[] PROGMEM = "\xB2\xB3";
static const char LFSTR_OFF[] PROGMEM = "\xB4\xB5";

keyball_t keyball = {
    .this_have_ball = false,
    .that_enable    = false,
    .that_have_ball = false,

    .this_motion = {0},
    .that_motion = {0},

    .cpi_value   = 0,
    .cpi_changed = false,

    .scroll_mode = false,
    .scroll_div  = 0,

    .pressing_keys = { BL, BL, BL, BL, BL, BL, 0 },
};

#ifdef AZ1UBALL_ENABLE
// ----------------------------------------------------------------
// keyball.this_have_ball は「split RPC negotiationで相手にthat_have_ballを
// 伝えるためのフラグ」として使う一方、公式コードでは同時に
// 「PMW3360を物理的に持っているか(pmw3360_cpi_set等を呼んでよいか)」の
// 判定にも使われている。
//
// AZ1UBALL構成ではこの2つの意味が両立しない(左手はnegotiation上は
// true(ボールがある)でありながら、PMW3360を物理的に持たないため
// pmw3360_cpi_set()を呼んではいけない)。
//
// そこで「PMW3360を物理的に持っているか」だけを別フラグとして分離する。
// keyball.hを変更せずに済むよう、ファイルスコープのstatic変数として持つ。
// ----------------------------------------------------------------
static bool pmw3360_physically_present = false;
#endif

// ----------------------------------------------------------------
// [デバッグ用] pmw3360_init()の戻り値をOLEDで確認するための一時変数。
// 動作確認後は削除して構わない。
// ----------------------------------------------------------------
static bool debug_pmw3360_init_result = false;

//////////////////////////////////////////////////////////////////////////////
// Hook points

__attribute__((weak)) void keyball_on_adjust_layout(keyball_adjust_t v) {}

//////////////////////////////////////////////////////////////////////////////
// Static utilities

// add16 adds two int16_t with clipping.
static int16_t add16(int16_t a, int16_t b) {
    int16_t r = a + b;
    if (a >= 0 && b >= 0 && r < 0) {
        r = 32767;
    } else if (a < 0 && b < 0 && r >= 0) {
        r = -32768;
    }
    return r;
}

// divmod16 divides *v by div, returns the quotient, and assigns the remainder
// to *v.
static int16_t divmod16(int16_t *v, int16_t div) {
    int16_t r = *v / div;
    *v -= r * div;
    return r;
}

// clip2int8 clips an integer fit into int8_t.
static inline int8_t clip2int8(int16_t v) {
    return (v) < -127 ? -127 : (v) > 127 ? 127 : (int8_t)v;
}

#ifdef OLED_ENABLE
static const char *format_4d(int8_t d) {
    static char buf[5] = {0}; // max width (4) + NUL (1)
    char        lead   = ' ';
    if (d < 0) {
        d    = -d;
        lead = '-';
    }
    buf[3] = (d % 10) + '0';
    d /= 10;
    if (d == 0) {
        buf[2] = lead;
        lead   = ' ';
    } else {
        buf[2] = (d % 10) + '0';
        d /= 10;
    }
    if (d == 0) {
        buf[1] = lead;
        lead   = ' ';
    } else {
        buf[1] = (d % 10) + '0';
        d /= 10;
    }
    buf[0] = lead;
    return buf;
}

static char to_1x(uint8_t x) {
    x &= 0x0f;
    return x < 10 ? x + '0' : x + 'a' - 10;
}
#endif

static void add_cpi(int8_t delta) {
    int16_t v = keyball_get_cpi() + delta;
    keyball_set_cpi(v < 1 ? 1 : v);
}

static void add_scroll_div(int8_t delta) {
    int8_t v = keyball_get_scroll_div() + delta;
    keyball_set_scroll_div(v < 1 ? 1 : v);
}

//////////////////////////////////////////////////////////////////////////////
// Pointing device driver

#if KEYBALL_MODEL == 46
void keyboard_pre_init_kb(void) {
    keyball.this_have_ball = pmw3360_init();
    keyboard_pre_init_user();
}
#endif

void pointing_device_driver_init(void) {
#if KEYBALL_MODEL != 46
    // ----------------------------------------------------------------
    // AZ1UBALLは左手側にのみ物理接続されている。
    // 右手: 従来通りPMW3360を初期化する。
    // 左手: PMW3360の代わりにAZ1UBALLを初期化する。
    //
    //       this_have_ballはPMW3360用のフラグに見えるが、実際には
    //       「split RPC negotiation(rpc_get_info)で相手に伝わり、
    //        相手側のthat_have_ballとして反映される」という重要な役割を持つ。
    //       housekeeping_task_kbはthat_have_ballがtrueの時だけ
    //       rpc_get_motion_invoke()/rpc_set_cpi_invoke()を呼ぶため、
    //       左手をfalseのままにすると右手マスター時にCPI同期とモーション
    //       RPCが両方止まってしまう(実機で発生した不具合の原因)。
    //
    //       そのため左手も this_have_ball = true にする。
    //       直後にreturnするので、この関数の下側にあるPMW3360専用の
    //       初期化ブロック(pmw3360_srom_upload/pmw3360_cpi_set)には
    //       左手では絶対に到達しない。
    //
    //       ただし this_have_ball=true 化により、keyball_set_cpi()内の
    //       "if (this_have_ball) pmw3360_cpi_set(...)" というガードも
    //       誤って左手で通過してしまう(左手はEEPROM読み込み等で
    //       keyball_set_cpi()が普通に呼ばれるため)。これを防ぐため、
    //       「PMW3360を物理的に持っているか」を示す専用フラグ
    //       pmw3360_physically_present を別途falseのまま維持する。
    // ----------------------------------------------------------------
#    ifdef AZ1UBALL_ENABLE
    if (is_keyboard_left()) {
        keyball.this_have_ball      = true;  // RPC negotiation用(右手にthat_have_ball=trueを伝える)
        pmw3360_physically_present  = false; // PMW3360操作(pmw3360_cpi_set等)は禁止のまま
        az1uball_init();
        return;
    }
#    endif
    keyball.this_have_ball = pmw3360_init();
    debug_pmw3360_init_result = keyball.this_have_ball; // [デバッグ用]
#    ifdef AZ1UBALL_ENABLE
    pmw3360_physically_present = keyball.this_have_ball; // 右手は従来通り連動させる
#    endif
#endif
    if (keyball.this_have_ball) {
#if defined(KEYBALL_PMW3360_UPLOAD_SROM_ID)
#    if KEYBALL_PMW3360_UPLOAD_SROM_ID == 0x04
        pmw3360_srom_upload(pmw3360_srom_0x04);
#    elif KEYBALL_PMW3360_UPLOAD_SROM_ID == 0x81
        pmw3360_srom_upload(pmw3360_srom_0x81);
#    else
#        error Invalid value for KEYBALL_PMW3360_UPLOAD_SROM_ID. Please choose 0x04 or 0x81 or disable it.
#    endif
#endif
        pmw3360_cpi_set(CPI_DEFAULT - 1);
    }
}

uint16_t pointing_device_driver_get_cpi(void) {
    return keyball_get_cpi();
}

void pointing_device_driver_set_cpi(uint16_t cpi) {
    keyball_set_cpi(cpi);
}

__attribute__((weak)) void keyball_on_apply_motion_to_mouse_move(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
#if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
    r->x = clip2int8(m->y);
    r->y = clip2int8(m->x);
    if (is_left) {
        r->x = -r->x;
        r->y = -r->y;
    }
#elif KEYBALL_MODEL == 46
    r->x = clip2int8(m->x);
    r->y = -clip2int8(m->y);
#else
#    error("unknown Keyball model")
#endif
    // clear motion
    m->x = 0;
    m->y = 0;
}

__attribute__((weak)) void keyball_on_apply_motion_to_mouse_scroll(keyball_motion_t *m, report_mouse_t *r, bool is_left) {
    // consume motion of trackball.
    int16_t div = 1 << (keyball_get_scroll_div() - 1);
    int16_t x = divmod16(&m->x, div);
    int16_t y = divmod16(&m->y, div);

    // apply to mouse report.
#if KEYBALL_MODEL == 61 || KEYBALL_MODEL == 39 || KEYBALL_MODEL == 147 || KEYBALL_MODEL == 44
    r->h = clip2int8(y);
    r->v = -clip2int8(x);
    if (is_left) {
        r->h = -r->h;
        r->v = -r->v;
    }
#elif KEYBALL_MODEL == 46
    r->h = clip2int8(x);
    r->v = clip2int8(y);
#else
#    error("unknown Keyball model")
#endif

    if (!is_left) {
    // Scroll snapping
#if KEYBALL_SCROLLSNAP_ENABLE == 1
    // Old behavior up to 1.3.2)
    uint32_t now = timer_read32();
    if (r->h != 0 || r->v != 0) {
        keyball.scroll_snap_last = now;
    } else if (TIMER_DIFF_32(now, keyball.scroll_snap_last) >= KEYBALL_SCROLLSNAP_RESET_TIMER) {
        keyball.scroll_snap_tension_h = 0;
    }
    if (abs(keyball.scroll_snap_tension_h) < KEYBALL_SCROLLSNAP_TENSION_THRESHOLD) {
        keyball.scroll_snap_tension_h += y;
        r->h = 0;
    }
#elif KEYBALL_SCROLLSNAP_ENABLE == 2
    // New behavior
    switch (keyball_get_scrollsnap_mode()) {
        case KEYBALL_SCROLLSNAP_MODE_VERTICAL:
            r->h = 0;
            break;
        case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL:
            r->v = 0;
            break;
        default:
            // pass by without doing anything
            break;
    }
#endif
}
}

static void motion_to_mouse(keyball_motion_t *m, report_mouse_t *r, bool is_left, bool as_scroll) {
    if (as_scroll) {
        keyball_on_apply_motion_to_mouse_scroll(m, r, is_left);
    } else {
        keyball_on_apply_motion_to_mouse_move(m, r, is_left);
    }
}

static inline bool should_report(void) {
    uint32_t now = timer_read32();
#if defined(KEYBALL_REPORTMOUSE_INTERVAL) && KEYBALL_REPORTMOUSE_INTERVAL > 0
    // throttling mouse report rate.
    static uint32_t last = 0;
    if (TIMER_DIFF_32(now, last) < KEYBALL_REPORTMOUSE_INTERVAL) {
        return false;
    }
    last = now;
#endif
#if defined(KEYBALL_SCROLLBALL_INHIVITOR) && KEYBALL_SCROLLBALL_INHIVITOR > 0
    if (TIMER_DIFF_32(now, keyball.scroll_mode_changed) < KEYBALL_SCROLLBALL_INHIVITOR) {
        keyball.this_motion.x = 0;
        keyball.this_motion.y = 0;
        keyball.that_motion.x = 0;
        keyball.that_motion.y = 0;
    }
#endif
    return true;
}

report_mouse_t pointing_device_driver_get_report(report_mouse_t rep) {
    // ----------------------------------------------------------------
    // AZ1UBALLは左手側にのみ物理接続されている。
    // 右手: 従来通りPMW3360からモーションを取得してthis_motionに加算する。
    // 左手: PMW3360の代わりにAZ1UBALLからモーションを取得してthis_motionに加算する。
    //       this_motionを共有することで、この先のrpc_get_motion_handler/invoke、
    //       motion_to_mouse()には一切手を加えずにそのまま運んでもらえる。
    // ----------------------------------------------------------------
#ifdef AZ1UBALL_ENABLE
    if (is_keyboard_left()) {
        int16_t az_x = 0, az_y = 0;
        bool    az_btn = false;
        az1uball_get_motion(&az_x, &az_y, &az_btn);
        // ----------------------------------------------------------------
        // keyball_on_apply_motion_to_mouse_scroll() は KEYBALL_MODEL==44 の場合
        // (is_left=true時):
        //   h = -(this_motion.y)
        //   v =  (this_motion.x)
        // という、PMW3360の物理的な取り付け向きに合わせたx/y入れ替え+符号反転を行う。
        //
        // az1uballはPMW3360と物理的な取り付け向きが異なるため、この変換を
        // そのまま受けると軸がズレる(実機で「→操作で上スクロール」として発現した)。
        //
        // this_motionへの加算時点で先にx/yを入れ替え、符号を補正しておくことで、
        // 最終的に h=az_x由来(左右), v=az_y由来(上下) という素直な対応にする。
        //   目標: h = az_x, v = az_y
        //   上の変換式に this_motion.x = az_y, this_motion.y = -az_x を入れると
        //     h = -(-az_x) = az_x  ✓
        //     v =  (az_y)          ✓
        //
        // az_btnは常にthis_motion.btnに直接書き込む(動きの有無に関わらず)。
        // これによりrpc_get_motion_handler(this_motion構造体をそのまま送る)経由で
        // 右マスター時もボタン状態が自動的に運ばれるようになる。
        // ----------------------------------------------------------------
        ATOMIC_BLOCK_FORCEON {
            keyball.this_motion.x   = add16(keyball.this_motion.x, az_y);
            keyball.this_motion.y   = add16(keyball.this_motion.y, -az_x);
            keyball.this_motion.btn = az_btn;
        }
    } else
#endif
    {
        // fetch from optical sensor.
        if (keyball.this_have_ball) {
            pmw3360_motion_t d = {0};
            if (pmw3360_motion_burst(&d)) {
                ATOMIC_BLOCK_FORCEON {
                    keyball.this_motion.x = add16(keyball.this_motion.x, d.x);
                    keyball.this_motion.y = add16(keyball.this_motion.y, d.y);
                }
            }
        }
    }
    // report mouse event, if keyboard is primary.
    if (is_keyboard_master() && should_report()) {
        // ----------------------------------------------------------------
        // this_motion / that_motion の scroll_mode 判定。
        //
        // ご要望の仕様:
        //   PMW3360(右手) : scroll_modeオフ→マウス, オン→スクロール(従来通り)
        //   AZ1UBALL(左手): 常にPMW3360と逆モード
        //                   (scroll_modeオフ→スクロール, オン→マウス)
        //
        // 両方が同じh/v(またはx/y)に書き込もうとすると、motion_to_mouseの
        // 呼び出し順(this→that)で後勝ちの上書きが発生してしまう
        // (scroll_modeオン時に両方スクロールにしていたら、az1uball側の
        //  書き込みでPMW3360側のh/vが上書きされる不具合が発生していた)。
        // az1uballとPMW3360を常に排他モードにすることで、片方はh/v、
        // もう片方はx/yに書き込むことになり、上書き競合そのものがなくなる。
        //
        // this_motionとthat_motionはそれぞれ「az1uball由来かPMW3360由来か」を
        // 判定する必要がある。
        //   this_motionがaz1uball由来  ⇔ 自分が左手 (is_keyboard_left())
        //   that_motionがaz1uball由来  ⇔ 自分が右手マスター (相手=左手=az1uball)
        //                                ⇔ !is_keyboard_left()
        // ----------------------------------------------------------------
#ifdef AZ1UBALL_ENABLE
        bool this_as_scroll = is_keyboard_left() ? !keyball.scroll_mode : keyball.scroll_mode;
        bool that_as_scroll = !is_keyboard_left() ? !keyball.scroll_mode : keyball.scroll_mode;
#else
        bool this_as_scroll = keyball.scroll_mode;
        bool that_as_scroll = keyball.scroll_mode ^ keyball.this_have_ball;
#endif
        motion_to_mouse(&keyball.this_motion, &rep, is_keyboard_left(), this_as_scroll);
        motion_to_mouse(&keyball.that_motion, &rep, !is_keyboard_left(), that_as_scroll);

#ifdef AZ1UBALL_ENABLE
        // ----------------------------------------------------------------
        // AZ1UBALLのクリック状態をrep.buttonsに反映する。
        //
        // this_motion/that_motionのどちらがaz1uball由来かは、this_as_scroll/
        // that_as_scrollの判定と同じ考え方(is_keyboard_left())で決まる。
        //   左手マスター: this_motionがaz1uball由来 → this_motion.btnを見る
        //   右手マスター: that_motionがaz1uball由来 → that_motion.btnを見る
        //
        // 重要: az_btnはレベル値(押されている間ずっとtrue)なので、押されて
        // いなければ明示的にビットを下ろす(&= ~MOUSE_BTN1)必要がある。
        // 単純に |= MOUSE_BTN1 だけだと、離した後もビットが立ったままになり
        // 「押されっぱなし」になる(実機で発生した不具合の原因)。
        // ----------------------------------------------------------------
        bool az_btn_now = is_keyboard_left() ? keyball.this_motion.btn : keyball.that_motion.btn;
        if (az_btn_now) {
            rep.buttons |= MOUSE_BTN6;
        } else {
            rep.buttons &= ~MOUSE_BTN6;
        }
#endif

        // store mouse report for OLED.
        keyball.last_mouse = rep;
    }
    return rep;
}

//////////////////////////////////////////////////////////////////////////////
// Split RPC

#ifdef SPLIT_KEYBOARD

static void rpc_get_info_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    keyball_info_t info = {
        .ballcnt = keyball.this_have_ball ? 1 : 0,
    };
    *(keyball_info_t *)out_data = info;
    keyball_on_adjust_layout(KEYBALL_ADJUST_SECONDARY);
}

static void rpc_get_info_invoke(void) {
    static bool     negotiated = false;
    static uint32_t last_sync  = 0;
    static int      round      = 0;
    uint32_t        now        = timer_read32();
    if (negotiated || TIMER_DIFF_32(now, last_sync) < KEYBALL_TX_GETINFO_INTERVAL) {
        return;
    }
    last_sync = now;
    round++;
    keyball_info_t recv = {0};
    if (!transaction_rpc_exec(KEYBALL_GET_INFO, 0, NULL, sizeof(recv), &recv)) {
        if (round < KEYBALL_TX_GETINFO_MAXTRY) {
            dprintf("keyball:rpc_get_info_invoke: missed #%d\n", round);
            return;
        }
    }
    negotiated             = true;
    keyball.that_enable    = true;
    keyball.that_have_ball = recv.ballcnt > 0;
    dprintf("keyball:rpc_get_info_invoke: negotiated #%d %d\n", round, keyball.that_have_ball);

    // split keyboard negotiation completed.

#    ifdef VIA_ENABLE
    // adjust VIA layout options according to current combination.
    uint8_t  layouts = (keyball.this_have_ball ? (is_keyboard_left() ? 0x02 : 0x01) : 0x00) | (keyball.that_have_ball ? (is_keyboard_left() ? 0x01 : 0x02) : 0x00);
    uint32_t curr    = via_get_layout_options();
    uint32_t next    = (curr & ~0x3) | layouts;
    if (next != curr) {
        via_set_layout_options(next);
    }
#    endif

    keyball_on_adjust_layout(KEYBALL_ADJUST_PRIMARY);
}

static void rpc_get_motion_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    *(keyball_motion_t *)out_data = keyball.this_motion;
    // clear motion
    keyball.this_motion.x = 0;
    keyball.this_motion.y = 0;
}

static void rpc_get_motion_invoke(void) {
    static uint32_t last_sync = 0;
    uint32_t        now       = timer_read32();
    if (TIMER_DIFF_32(now, last_sync) < KEYBALL_TX_GETMOTION_INTERVAL) {
        return;
    }
    keyball_motion_t recv = {0};
    if (transaction_rpc_exec(KEYBALL_GET_MOTION, 0, NULL, sizeof(recv), &recv)) {
        keyball.that_motion.x = add16(keyball.that_motion.x, recv.x);
        keyball.that_motion.y = add16(keyball.that_motion.y, recv.y);
#ifdef AZ1UBALL_ENABLE
        // btnはx/yと違い「累積差分」ではなく「現在押されているかどうか」の
        // レベル値なので、加算ではなく最新の受信値でそのまま上書きする。
        keyball.that_motion.btn = recv.btn;
#endif
    }
#ifdef AZ1UBALL_ENABLE
    else {
        // ----------------------------------------------------------------
        // RPC受信自体が失敗した場合(瞬断など)。x/yは前回値を保持して問題ないが、
        // btnをそのまま放置すると「押した瞬間に瞬断→離してもtrueのまま残り、
        // 押されっぱなしになる」危険がある。安全側としてfalseに倒す。
        // ----------------------------------------------------------------
        keyball.that_motion.btn = false;
    }
#endif
    last_sync = now;
    return;
}

static void rpc_set_cpi_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    keyball_set_cpi(*(keyball_cpi_t *)in_data);
}

static void rpc_set_cpi_invoke(void) {
    if (!keyball.cpi_changed) {
        return;
    }
    keyball_cpi_t req = keyball.cpi_value;
    if (!transaction_rpc_send(KEYBALL_SET_CPI, sizeof(req), &req)) {
        return;
    }
    keyball.cpi_changed = false;
}

#endif

//////////////////////////////////////////////////////////////////////////////
// OLED utility

#ifdef OLED_ENABLE
// clang-format off
const char PROGMEM code_to_name[] = {
    'a', 'b', 'c', 'd', 'e', 'f',  'g', 'h', 'i',  'j',
    'k', 'l', 'm', 'n', 'o', 'p',  'q', 'r', 's',  't',
    'u', 'v', 'w', 'x', 'y', 'z',  '1', '2', '3',  '4',
    '5', '6', '7', '8', '9', '0',  'R', 'E', 'B',  'T',
    '_', '-', '=', '[', ']', '\\', '#', ';', '\'', '`',
    ',', '.', '/',
};
// clang-format on
#endif

void keyball_oled_render_ballinfo(void) {
#ifdef OLED_ENABLE
    oled_write_P(PSTR("Ball\xB1"), false);
    oled_write(format_4d(keyball.last_mouse.x), false);
    oled_write(format_4d(keyball.last_mouse.y), false);
    oled_write(format_4d(keyball.last_mouse.h), false);
    oled_write(format_4d(keyball.last_mouse.v), false);

    oled_write_P(PSTR("    \xB1\xBC\xBD"), false);
    oled_write(format_4d(keyball_get_cpi()) + 1, false);
    oled_write_P(PSTR("00 "), false);

#if 1 && KEYBALL_SCROLLSNAP_ENABLE == 2
    switch (keyball_get_scrollsnap_mode()) {
        case KEYBALL_SCROLLSNAP_MODE_VERTICAL:
            oled_write_P(PSTR("VT"), false);
            break;
        case KEYBALL_SCROLLSNAP_MODE_HORIZONTAL:
            oled_write_P(PSTR("HO"), false);
            break;
        default:
            oled_write_P(PSTR("\xBE\xBF"), false);
            break;
    }
#else
    oled_write_P(PSTR("\xBE\xBF"), false);
#endif
    if (keyball.scroll_mode) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    oled_write_P(PSTR(" \xC0\xC1"), false);
    oled_write_char('0' + keyball_get_scroll_div(), false);

    // ----------------------------------------------------------------
    // [デバッグ用] PMW3360初期化結果を表示する。動作確認後は削除して構わない。
    //
    // 表示: " P{0/1}T{0/1}"
    //   P = debug_pmw3360_init_result (pmw3360_init()の生の戻り値)
    //   T = keyball.this_have_ball    (RPC negotiation用フラグ)
    //
    // 右手でP=0なら pmw3360_init() 自体が失敗している
    // (SPI通信不良・配線・CSピン設定などハードウェア/設定起因の可能性が高い)。
    // 右手でP=1なのにボールが動かない場合は、motion_burst読み取り側や
    // motion_to_mouse以降のロジックを疑う。
    // ----------------------------------------------------------------
    oled_write_P(PSTR(" P"), false);
    oled_write_char(debug_pmw3360_init_result ? '1' : '0', false);
    oled_write_P(PSTR("T"), false);
    oled_write_char(keyball.this_have_ball ? '1' : '0', false);
#endif
}

void keyball_oled_render_ballsubinfo(void) {
#ifdef OLED_ENABLE
#endif
}

void keyball_oled_render_keyinfo(void) {
#ifdef OLED_ENABLE
    oled_write_P(PSTR("Key \xB1"), false);

    oled_write_char('\xB8', false);
    oled_write_char(to_1x(keyball.last_pos.row), false);
    oled_write_char('\xB9', false);
    oled_write_char(to_1x(keyball.last_pos.col), false);

    oled_write_P(PSTR("\xBA\xBB"), false);
    oled_write_char(to_1x(keyball.last_kc >> 4), false);
    oled_write_char(to_1x(keyball.last_kc), false);

    oled_write_P(PSTR("  "), false);
    oled_write(keyball.pressing_keys, false);
#endif
}

void keyball_oled_render_layerinfo(void) {
#ifdef OLED_ENABLE
    oled_write_P(PSTR("L\xB6\xB7r\xB1"), false);
    for (uint8_t i = 1; i < 8; i++) {
        oled_write_char((layer_state_is(i) ? to_1x(i) : BL), false);
    }
    oled_write_char(' ', false);

#    ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
    oled_write_P(PSTR("\xC2\xC3"), false);
    if (get_auto_mouse_enable()) {
        oled_write_P(LFSTR_ON, false);
    } else {
        oled_write_P(LFSTR_OFF, false);
    }

    oled_write(format_4d(get_auto_mouse_timeout() / 10) + 1, false);
    oled_write_char('0', false);
#    else
    oled_write_P(PSTR("\xC2\xC3\xB4\xB5 ---"), false);
#    endif
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Public API functions

bool keyball_get_scroll_mode(void) {
    return keyball.scroll_mode;
}

void keyball_set_scroll_mode(bool mode) {
    if (mode != keyball.scroll_mode) {
        keyball.scroll_mode_changed = timer_read32();
    }
    keyball.scroll_mode = mode;
}

keyball_scrollsnap_mode_t keyball_get_scrollsnap_mode(void) {
#if KEYBALL_SCROLLSNAP_ENABLE == 2
    return keyball.scrollsnap_mode;
#else
    return 0;
#endif
}

void keyball_set_scrollsnap_mode(keyball_scrollsnap_mode_t mode) {
#if KEYBALL_SCROLLSNAP_ENABLE == 2
    keyball.scrollsnap_mode = mode;
#endif
}

uint8_t keyball_get_scroll_div(void) {
    return keyball.scroll_div == 0 ? KEYBALL_SCROLL_DIV_DEFAULT : keyball.scroll_div;
}

void keyball_set_scroll_div(uint8_t div) {
    keyball.scroll_div = div > SCROLL_DIV_MAX ? SCROLL_DIV_MAX : div;
}

uint8_t keyball_get_cpi(void) {
    return keyball.cpi_value == 0 ? CPI_DEFAULT : keyball.cpi_value;
}

void keyball_set_cpi(uint8_t cpi) {
    if (cpi > CPI_MAX) {
        cpi = CPI_MAX;
    }
    keyball.cpi_value   = cpi;
    keyball.cpi_changed = true;
    // ----------------------------------------------------------------
    // pmw3360_cpi_set()はPMW3360を物理的に持っている側でしか呼んではいけない。
    // AZ1UBALL_ENABLE時はkeyball.this_have_ballがRPC negotiation用に
    // 左手でもtrueになっているため、代わりにpmw3360_physically_presentで判定する。
    // AZ1UBALL_ENABLE未定義時は公式のthis_have_ballのままでよい。
    // ----------------------------------------------------------------
#ifdef AZ1UBALL_ENABLE
    if (pmw3360_physically_present) {
        pmw3360_cpi_set(cpi == 0 ? CPI_DEFAULT - 1 : cpi - 1);
    }
#else
    if (keyball.this_have_ball) {
        pmw3360_cpi_set(cpi == 0 ? CPI_DEFAULT - 1 : cpi - 1);
    }
#endif
}

//////////////////////////////////////////////////////////////////////////////
// Keyboard hooks

void keyboard_post_init_kb(void) {
#ifdef SPLIT_KEYBOARD
    // register transaction handlers on secondary.
    if (!is_keyboard_master()) {
        transaction_register_rpc(KEYBALL_GET_INFO, rpc_get_info_handler);
        transaction_register_rpc(KEYBALL_GET_MOTION, rpc_get_motion_handler);
        transaction_register_rpc(KEYBALL_SET_CPI, rpc_set_cpi_handler);
    }
#endif

    // read keyball configuration from EEPROM
    if (eeconfig_is_enabled()) {
        keyball_config_t c = {.raw = eeconfig_read_kb()};
        keyball_set_cpi(c.cpi);
        keyball_set_scroll_div(c.sdiv);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
        set_auto_mouse_enable(c.amle);
        set_auto_mouse_timeout(c.amlto == 0 ? AUTO_MOUSE_TIME : (c.amlto + 1) * AML_TIMEOUT_QU);
#endif
#if KEYBALL_SCROLLSNAP_ENABLE == 2
        keyball_set_scrollsnap_mode(c.ssnap);
#endif
    }

    keyball_on_adjust_layout(KEYBALL_ADJUST_PENDING);
    keyboard_post_init_user();
}

#if SPLIT_KEYBOARD
void housekeeping_task_kb(void) {
    if (is_keyboard_master()) {
        rpc_get_info_invoke();
        if (keyball.that_have_ball) {
            rpc_get_motion_invoke();
            rpc_set_cpi_invoke();
        }
    }
}
#endif

static void pressing_keys_update(uint16_t keycode, keyrecord_t *record) {
    // Process only valid keycodes.
    if (keycode >= 4 && keycode < 57) {
        char value = pgm_read_byte(code_to_name + keycode - 4);
        char where = BL;
        if (!record->event.pressed) {
            // Swap `value` and `where` when releasing.
            where = value;
            value = BL;
        }
        // Rewrite the last `where` of pressing_keys to `value` .
        for (int i = 0; i < KEYBALL_OLED_MAX_PRESSING_KEYCODES; i++) {
            if (keyball.pressing_keys[i] == where) {
                keyball.pressing_keys[i] = value;
                break;
            }
        }
    }
}

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
bool is_mouse_record_kb(uint16_t keycode, keyrecord_t* record) {
    switch (keycode) {
        case SCRL_MO:
            return true;
    }
    return is_mouse_record_user(keycode, record);
}
#endif

bool process_record_kb(uint16_t keycode, keyrecord_t *record) {
    // store last keycode, row, and col for OLED
    keyball.last_kc  = keycode;
    keyball.last_pos = record->event.key;

    pressing_keys_update(keycode, record);

    if (!process_record_user(keycode, record)) {
        return false;
    }

    // strip QK_MODS part.
    if (keycode >= QK_MODS && keycode <= QK_MODS_MAX) {
        keycode &= 0xff;
    }

    switch (keycode) {
#ifndef MOUSEKEY_ENABLE
        // process KC_MS_BTN1~8 by myself
        // See process_action() in quantum/action.c for details.
        case KC_MS_BTN1 ... KC_MS_BTN8: {
            extern void register_mouse(uint8_t mouse_keycode, bool pressed);
            register_mouse(keycode, record->event.pressed);
            // to apply QK_MODS actions, allow to process others.
            return true;
        }
#endif

        case SCRL_MO:
            keyball_set_scroll_mode(record->event.pressed);
            // process_auto_mouse may use this in future, if changed order of
            // processes.
            return true;
    }

    // process events which works on pressed only.
    if (record->event.pressed) {
        switch (keycode) {
            case KBC_RST:
                keyball_set_cpi(0);
                keyball_set_scroll_div(0);
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                set_auto_mouse_enable(false);
                set_auto_mouse_timeout(AUTO_MOUSE_TIME);
#endif
                break;
            case KBC_SAVE: {
                keyball_config_t c = {
                    .cpi   = keyball.cpi_value,
                    .sdiv  = keyball.scroll_div,
#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
                    .amle  = get_auto_mouse_enable(),
                    .amlto = (get_auto_mouse_timeout() / AML_TIMEOUT_QU) - 1,
#endif
#if KEYBALL_SCROLLSNAP_ENABLE == 2
                    .ssnap = keyball_get_scrollsnap_mode(),
#endif
                };
                eeconfig_update_kb(c.raw);
            } break;

            case CPI_I100:
                add_cpi(1);
                break;
            case CPI_D100:
                add_cpi(-1);
                break;
            case CPI_I1K:
                add_cpi(10);
                break;
            case CPI_D1K:
                add_cpi(-10);
                break;

            case SCRL_TO:
                keyball_set_scroll_mode(!keyball.scroll_mode);
                break;
            case SCRL_DVI:
                add_scroll_div(1);
                break;
            case SCRL_DVD:
                add_scroll_div(-1);
                break;

#if KEYBALL_SCROLLSNAP_ENABLE == 2
            case SSNP_HOR:
                keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_HORIZONTAL);
                break;
            case SSNP_VRT:
                keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_VERTICAL);
                break;
            case SSNP_FRE:
                keyball_set_scrollsnap_mode(KEYBALL_SCROLLSNAP_MODE_FREE);
                break;
#endif

#ifdef POINTING_DEVICE_AUTO_MOUSE_ENABLE
            case AML_TO:
                set_auto_mouse_enable(!get_auto_mouse_enable());
                break;
            case AML_I50:
                {
                    uint16_t v = get_auto_mouse_timeout() + 50;
                    set_auto_mouse_timeout(MIN(v, AML_TIMEOUT_MAX));
                }
                break;
            case AML_D50:
                {
                    uint16_t v = get_auto_mouse_timeout() - 50;
                    set_auto_mouse_timeout(MAX(v, AML_TIMEOUT_MIN));
                }
                break;
#endif

            default:
                return true;
        }
        return false;
    }

    return true;
}

// Disable functions keycode_config() and mod_config() in keycode_config.c to
// reduce size.  These functions are provided for customizing magic keycode.
// These two functions are mostly unnecessary if `MAGIC_KEYCODE_ENABLE = no` is
// set.
//
// If `MAGIC_KEYCODE_ENABLE = no` and you want to keep these two functions as
// they are, define the macro KEYBALL_KEEP_MAGIC_FUNCTIONS.
//
// See: https://docs.qmk.fm/#/squeezing_avr?id=magic-functions
//
#if !defined(MAGIC_KEYCODE_ENABLE) && !defined(KEYBALL_KEEP_MAGIC_FUNCTIONS)

uint16_t keycode_config(uint16_t keycode) {
    return keycode;
}

uint8_t mod_config(uint8_t mod) {
    return mod;
}

#endif