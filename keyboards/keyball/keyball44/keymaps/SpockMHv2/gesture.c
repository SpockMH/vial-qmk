#include "quantum.h"
#include <stdlib.h>
// ===== rotation detect (sampling + time limit) =====
#define ROT_THRESH   10      // ← 積算後なので少し大きめ
#define ROT_TERM     350
#define ROT_SAMPLE   10     // ← 追加：評価周期(ms)

static int8_t  last_dir = 0;
static int8_t  rot_step = 0;


static int8_t acc_x = 0;   // ← 積算
static int8_t acc_y = 0;

static uint16_t rot_timer = 0;
static uint16_t sample_timer = 0;

enum {
    DIR_NONE,
    DIR_R,
    DIR_D,
    DIR_L,
    DIR_U
};

static int8_t get_dir(int8_t x, int8_t y){
    if(abs(x) > abs(y)){
        if(x > ROT_THRESH)  return DIR_R;
        if(x < -ROT_THRESH) return DIR_L;
    }else{
        if(y > ROT_THRESH)  return DIR_D;
        if(y < -ROT_THRESH) return DIR_U;
    }
    return DIR_NONE;
}



void mouse_gesture_task(uint8_t x,uint8_t y){

    /* ===== 毎回積算 ===== */
    acc_x = qadd8(acc_x,x);
    acc_y = qadd8(acc_y,y);

    /* ===== 10ms未満は何もしない ===== */
    if(timer_elapsed(sample_timer) < ROT_SAMPLE)
        return;

    sample_timer = timer_read();

    int8_t dir = get_dir(acc_x, acc_y);

    /* 積算リセット（ここ重要） */
    acc_x = 0;
    acc_y = 0;

    if(dir == DIR_NONE || dir == last_dir) return;

    /* ===== タイムアウト ===== */
    if(rot_timer && timer_elapsed(rot_timer) > ROT_TERM){
        rot_step = 0;
        rot_timer = 0;
    }

    if(!rot_timer) rot_timer = timer_read();

    // 時計回り
    if(
        (last_dir==DIR_R && dir==DIR_D) ||
        (last_dir==DIR_D && dir==DIR_L) ||
        (last_dir==DIR_L && dir==DIR_U) ||
        (last_dir==DIR_U && dir==DIR_R)
    ){
        rot_step++;
    }
    // 反時計回り
    else if(
        (last_dir==DIR_R && dir==DIR_U) ||
        (last_dir==DIR_U && dir==DIR_L) ||
        (last_dir==DIR_L && dir==DIR_D) ||
        (last_dir==DIR_D && dir==DIR_R)
    ){
        rot_step--;
    }

    last_dir = dir;

    /* ===== 1周 ===== */
    if(rot_step >= 4){
        tap_code16(0x709);
        rot_step = 0;
        rot_timer = 0;
    } else if(rot_step <= -4){
        tap_code16(0x70A);
        rot_step = 0;
        rot_timer = 0;
    }

}
