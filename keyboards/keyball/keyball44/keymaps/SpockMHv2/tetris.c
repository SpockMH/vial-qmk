#include "quantum.h"
#include "oled_driver.h"
#include <stdlib.h>
#include <string.h>
#include "progmem.h"

#ifdef OLED_ENABLE

/* =========================================================
   Keyball OLED Tetris 128x32

   【設計思想まとめ】
   ・OLEDはピクセル単位だと扱いづらい
       → CELL(3px) の「論理マス」に変換して扱う
   ・ロジック(update) と 描画(render) を完全分離
   ・状態管理はステートマシン方式
   ・ゲームオーバー時は自動リセット

   呼び出し：
      init   → 起動時1回
      update → housekeeping_task（ロジック）
      render → oled_task（描画のみ）
   ========================================================= */


/* =========================================================
   画面レイアウト設定
   ========================================================= */

#define OLED_H 120
#define OLED_W 32

#define OLED_BYTES (OLED_W * OLED_H / 8)

static char oled_buf[OLED_BYTES];       // ★ 画面1枚分バッファ

#define CELL   4     // 1マス=3px

#define FIELD_H 28
#define FIELD_W 8

static uint16_t field[FIELD_H]; // 10bit使用
static uint8_t hi_score;
static uint8_t score = 0;

static const char PROGMEM tetris_logo[32] = {
    // T (6px)
    0x01,0x01,0xFF,0x01,0x01,0x00,
    // E (6px)
    0xFF,0x89,0x89,0x89,0x89,0x00,
    // T (6px)
    0x01,0x01,0xFF,0x01,0x01,0x00,
    // R (6px)
    0xFF,0x11,0x31,0x51,0x8E,0x00,
    // I (4px)
    0x81,0xFF,0x81,0x00,
    // S (4px)
    0x8E,0x91,0x91,0x61
};
/* ========= テトリミノ（16bit圧縮） ========= */

static const uint16_t PROGMEM blocks[7][4] = {
{0x0F00,0x2222,0x0F00,0x4444}, // I
{0x0660,0x0660,0x0660,0x0660}, // O
{0x0720,0x2620,0x2700,0x2320}, // T
{0x0360,0x2310,0x0360,0x2310}, // S
{0x0630,0x1320,0x0630,0x1320}, // Z
{0x0710,0x3220,0x4700,0x2230}, // J
{0x0740,0x2230,0x1700,0x6220}  // L
};

static const uint8_t PROGMEM font3x5[10][5] = {
{0b111,0b101,0b101,0b101,0b111}, //0
{0b010,0b110,0b010,0b010,0b111}, //1
{0b111,0b001,0b111,0b100,0b111}, //2
{0b111,0b001,0b111,0b001,0b111}, //3
{0b101,0b101,0b111,0b001,0b001}, //4
{0b111,0b100,0b111,0b001,0b111}, //5
{0b111,0b100,0b111,0b101,0b111}, //6
{0b111,0b001,0b001,0b001,0b001}, //7
{0b111,0b101,0b111,0b101,0b111}, //8
{0b111,0b101,0b111,0b001,0b111}  //9
};

/* 現在落下中のミノ */
typedef struct {
    int8_t x;
    int8_t y;
    uint8_t type;
    uint8_t rot;
} piece_t;

static piece_t cur;

static uint16_t fall_timer = 0;


///////////////////////////////////////////////////////////////////////////////
//OLED
///////////////////////////////////////////////////////////////////////////////
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

// 非常にシンプルな乱数生成関数
static uint8_t rand_basic(void) {
    static uint8_t seed = 0;
    seed = 89 * seed + 111; // 定数による次の値の生成
    return seed;
}

/* =========================================================
   ★ RAW描画ユーティリティ
   ========================================================= */

/* 1ピクセルON */
static inline void pset(int x,int y){
    if(x < 0 || x >= OLED_W) return;
    if(y < 0 || y >= OLED_H) return;

    uint16_t idx = x + (y >> 3) * OLED_W;

    if(idx >= OLED_BYTES) return;  // ★ 完全保険

    oled_buf[idx] |= (1 << (y & 7));
}

/* 3x3セル描画 */
static void draw_cell(int gx,int gy){
    int px = gx*CELL;
    int py = gy*CELL;
    if(px < 0 || py < 0) return;
    if(px + CELL > OLED_W) return;
    if(py + CELL > OLED_H) return;

    for(int dy=0;dy<CELL;dy++)
        for(int dx=0;dx<CELL;dx++)
            pset(px+dx,py+dy);
}

static void draw_separator(void){
    int y = 112;

    for(int x=0; x<OLED_W; x++)
        pset(x, y);
}


static inline void draw_digit(int x,int y,int n){
    if(n < 0 || n > 9) return;

    for(uint8_t row=0; row<5; row++){
        uint8_t line = pgm_read_byte(&font3x5[n][row]);

        for(uint8_t col=0; col<3; col++){
            if(line & (1<<(2-col)))
                pset(x+col, y+row);
        }
    }
}


/* =========================================================
    ゲームロジック
   ========================================================= */


static void draw_score(void){

    int base_y = 115;  // 区切り線の下

    uint8_t temp = score;
    int x = 28;
    uint8_t digit;
    if(temp == 0){
        draw_digit(x, base_y, 0);
    } else {
        while(temp>0) {
            digit = temp%10;
            draw_digit(x, base_y, digit);
            temp/=10;
            x-=4;
        }
    }

    temp = hi_score;
    x = 12;

    if(temp == 0){
        draw_digit(x, base_y, 0);
    } else {
        while(temp>0) {
            digit = temp%10;
            draw_digit(x, base_y, digit);
            temp/=10;
            x-=4;
        }
    }
}

/* 衝突判定
   「この位置に置いたら当たる？」を純粋判定 */
static bool collide(int nx,int ny,uint8_t type,uint8_t rot){

    uint16_t s = pgm_read_word(&blocks[type][rot]);

    for(int y=0;y<4;y++)
    for(int x=0;x<4;x++)
        if(s & (1<<(15-(y*4+x)))){

            int fx=nx+x;
            int fy=ny+y;

            if(fx<0||fx>=FIELD_W||fy>=FIELD_H) return true;
            if(fy>=0 && (field[fy]&(1<<fx))) return true;
        }

    return false;
}

static void merge_piece(void){

    uint16_t s = pgm_read_word(&blocks[cur.type][cur.rot]);

    for(int y=0;y<4;y++)
    for(int x=0;x<4;x++)
        if(s&(1<<(15-(y*4+x)))){

            int fx=cur.x+x;
            int fy=cur.y+y;

            if(fx>=0&&fx<FIELD_W&&fy>=0&&fy<FIELD_H)
                field[fy]|=(1<<fx);
        }
}

/* 盤面初期化＝新ゲーム開始 */
static void reset_game(void){
    memset(field,0,sizeof(field));
}

/* 新しいミノ生成
   生成時に衝突したら = 既に詰んでいる → GAME OVER */
static void spawn_piece(void){

    cur.type = rand_basic()%7;
    cur.rot  = 0;
    cur.x    = FIELD_W/2-2;
    cur.y    = 0;

    /* ここがゲームオーバー判定 */
    if(collide(cur.x,cur.y,cur.type,cur.rot)){
        if(hi_score<score){
            hi_score=score;
            set_hi_score(score);
        }
        reset_game();
        score=0;
    }
}

/* ライン消去 */
static void clear_lines(void){
    const uint16_t FULL = (1 << FIELD_W) - 1;  // 例: 0b0000001111111111
    for(int y = FIELD_H - 1; y >= 0; y--){

        if((field[y] & FULL) == FULL){

            // 1段ずつ上から詰める
            for(int ty = y; ty > 0; ty--)
                field[ty] = field[ty - 1];

            field[0] = 0;  // 最上段を空に
            y++; // 同じ段を再チェック（連続消し対応）
            score+=1;
        }
    }
}


/* =========================================================
    ★ render（RAW一発描画）
   ========================================================= */

void tetris_render(void){

    memset(oled_buf,0,OLED_BYTES);

    /* 固定ブロック */
    for(int y=0;y<FIELD_H;y++)
        for(int x=0;x<FIELD_W;x++)
            if(field[y]&(1<<x))
                draw_cell(x,y);

    /* 落下中 */
    uint16_t s=pgm_read_word(&blocks[cur.type][cur.rot]);

    for(int y=0;y<4;y++)
    for(int x=0;x<4;x++)
        if(s&(1<<(15-(y*4+x))))
            draw_cell(cur.x+x,cur.y+y);
    
    draw_separator();
    draw_score();
    oled_set_cursor(0, 1);
    oled_write_raw(oled_buf, OLED_BYTES);

}


/* =========================================================
    update（ロジックのみ）
   ========================================================= */
void tetris_update(void){

    if(++fall_timer <400) return;
    fall_timer = 0;

    if(!collide(cur.x,cur.y+1,cur.type,cur.rot))
        cur.y++;
    else{
        merge_piece();
        clear_lines();
        spawn_piece();
    }
    
}

/* ========================================================= */
void tetris_init(void){
    char logo_buf[sizeof(tetris_logo)];

    for(uint8_t i=0;i<sizeof(tetris_logo);i++){
        logo_buf[i] = pgm_read_byte(&tetris_logo[i]);
    }
    oled_write_raw(logo_buf, sizeof(logo_buf));
    hi_score = get_hi_score();
    reset_game();
    spawn_piece();
}

// QMKで呼ばれるユーザー描画関数の実装（ここで粒を降らせてる）
void oledkit_render_info_user(void) {
    tetris_update();
    tetris_render();
}


void tetris_move(int8_t x, int8_t y,int8_t rot){

    uint8_t next = (cur.rot+rot)&3;

    if(!collide(cur.x+x,cur.y+y,cur.type,next)){
        cur.x   += x;
        cur.y   += y;
        cur.rot = next;        
    }

}


void tetris_hard_drop(void){
    while(!collide(cur.x,cur.y+1,cur.type,cur.rot))
        cur.y++;

    merge_piece();
    spawn_piece();
}

#endif
