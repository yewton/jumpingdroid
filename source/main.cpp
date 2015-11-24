#include <gba.h>
#include <stdlib.h>
#include <stdio.h>
#include <cstdarg>
#include "vba.h"
#include "sprites.h"
#include "Sprite.hpp"

namespace {

/* フレーム数 */
u16 frame;

const u16 BUFSIZE = 1024;

/**
 * VBA のデバッグコンソールに出力する
 * @param const char* s 出力する文字列
 */
void cprintf(const char* format, ...) {
    va_list ap;
    char buf[BUFSIZE];

    va_start(ap, format);
    vsnprintf(buf, BUFSIZE, format, ap);
    va_end(ap);

    vbalog(buf);
}

}

int main() {
    /* ドロイド君の x, y 座標 */
    s16 dx = 120, dy = 120;
    /* りんごの x, y 座標 */
    const s16 ax = 160, ay = 120;
    /* 窓の x, y 座標 */
    const s16 wx = 40, wy = 40;
    /* タイルのベースアドレス */
    u16* tile = (u16*)TILE_BASE_ADR(0);
    /*
     * ドロイド君の状態。
     * 0 => 待機
     * 1 => ジャンプ準備中
     * 2 => ジャンプ中
     */
    int state = 0;
    /* 歩き状態 (0, 1, 2) */
    int wstate = 0;
    /* ドロイド君の y 方向の速度 */
    float vy = 0;
    /* フレーム数用の変数 */
    u16 f = 0;
    /* 表示するキャラクタ */
    u16 ch = 0;
    /* キー状態取得用変数 */
    u16 kd = 0;
    u16 kdr = 0;
    u16 ku = 0;
    u16 kh = 0;
    /* 汎用変数 */
    u16 i, xx, yy;

    /* フレーム数初期化 */
    frame = 0;

    /* 割り込みの初期化 */
    irqInit();
    /*
     * VBLANK 割り込みを有効化
     * これによって VBlankIntrWait() が使えるようになる
     */
    irqEnable(IRQ_VBLANK);

    /*
     * モードの設定。
     * MODE 0、スプライト有効化、BG0 有効化
     */
    SetMode(MODE_0 | OBJ_ENABLE | BG0_ON);

    /* スプライトのメモリ領域(OAM)にスプライトデータをコピー */
    GbaGraphics::setSpriteData(spritesTiles, spritesTilesLen / 2);

    /* スプライトと BG のパレットのメモリ領域にパレットデータをコピー */
    GbaGraphics::setSpritePalette(spritesPal, 16);
    GbaGraphics::setBGPalette(spritesPal, 16);
    GbaGraphics::setBGColor(RGB5(15,15,31));

    /* スプライトの初期化 */
    GbaGraphics::initSprites();

    /* ドロイド君の準備 */
    GbaGraphics::Sprite droid(0, 0, Sprite_16x16);
    droid.setPosition(dx, dy);
    droid.draw();

    /* りんごの準備 */
    GbaGraphics::Sprite apple(1, (2 * 32), Sprite_16x16);
    apple.setPosition(ax, ay);
    apple.draw();

    /* 窓の準備 */
    GbaGraphics::Sprite window(2, 2 + (2 * 32), Sprite_16x16);
    window.setPosition(wx, wy);
    window.draw();

    /* BG 0 の設定 */
    BGCTRL[0] = BG_MAP_BASE(31) | BG_16_COLOR | BG_SIZE_0 | TILE_BASE(0) | BG_PRIORITY(0);

    /* BG0 を初期化 */
    for ( xx = 0; xx < 32; xx++ ) {
        for ( yy = 0; yy < 32; yy++ ) {
            MAP[31][yy][xx] = 6 * 32;
        }
    }

    /* 画像をタイルにコピー */
    for ( i = 0; i < spritesTilesLen / 2; i++ ) {
        tile[i] = spritesTiles[i];
    }

    /* BG0 をセット */
    MAP[31][17][0] = 5 * 32;
    MAP[31][17][29] = 2 + 5 * 32;
    for ( i = 1; i < 29; i++ ) {
        MAP[31][17][i] = 1 + 5 * 32;
    }
    for ( xx = 0; xx < 30; xx++ ) {
        for ( yy = 18; yy < 32; yy++ ) {
            MAP[31][yy][xx] = 3 + 5 * 32;
        }
    }

    // コンソール出力のテスト
    cprintf("droid(%d, %d), apple(%d, %d), window(%d, %d), %s", dx, dy, ax, ay, wx, wy, "console test.");

    /* メインループ */
    while (1) {
        /* VBLANK 割り込み待ち */
        VBlankIntrWait();
        /* フレーム数カウント */
        frame += 1;
        /* キー状態取得 */
        scanKeys();
        kd = keysDown();
        kdr = keysDownRepeat();
        ku = keysUp();
        kh = keysHeld();

        switch(state) {
        case 0:
            /* 待機中 */
            if( (kd & KEY_UP) ) {
                state = 1;
                f = 0;
                ch = 0;
                break;
            }
            if( (kh & KEY_LEFT)  ) {
                dx--;
                droid.setHFlip(true);
            }
            if( (kh & KEY_RIGHT) ) {
                dx++;
                droid.setHFlip(false);
            }
            if( dx < -16 ) {
                dx = SCREEN_WIDTH;
            }
            if( kd & ( KEY_LEFT | KEY_RIGHT ) ) {
                wstate = 0;
                f = 0;
            }
            if( ku & ( KEY_LEFT | KEY_RIGHT ) ) {
                ch = 0;
            }
            if ( SCREEN_WIDTH < dx ) {
                dx = -16;
            }

            if ( kh & ( KEY_LEFT | KEY_RIGHT ) ) {
                /* 歩きモーション */
                if ( 5 < f++ ) {
                    switch ( wstate ) {
                    case 0:
                        wstate = 1;
                        ch = 2;
                        break;
                    case 1:
                        wstate = 2;
                        ch = 0;
                        break;
                    case 2:
                        wstate = 3;
                        ch = 4;
                        break;
                    default:
                        wstate = 0;
                        ch = 0;
                        break;
                    }
                    f = 0;
                }
            }

            if ( dy == (ay - 13) ) {
                if ( !(((ax - 11) < dx) &&
                       (dx < (ax + 11))) ) {
                    /* りんごから落ちる */
                    vy = -0.;
                    state = 2;
                    wstate = 0;
                    break;
                }
            }
            droid.setCharacter(ch);
            droid.setPosition(dx, dy);
            break;
        case 1:
        case 3:
            /* ジャンプ準備 */
            droid.setCharacter(6);
            if( 3 < f++ ) {
                vy = 4.;
                if ( 1 == state ) state = 2;
                else state = 4;
            }
            break;
        case 2:
        case 4:
            /* ジャンプ中 */
            if( (kd & KEY_UP) ) {
                /* 二段ジャンプ */
                if ( state == 2 ) {
                    state = 3;
                    f = 0;
                    break;
                }
            }
            if( kh & KEY_LEFT ) {
                dx--;
                droid.setHFlip(true);
            }
            if( kh & KEY_RIGHT ) {
                dx++;
                droid.setHFlip(false);
            }
            if( dx < -16 ) {
                dx = SCREEN_WIDTH;
            }
            if ( SCREEN_WIDTH < dx ) {
                dx = -16;
            }
            if( (0.5 < vy) && ( kh & KEY_UP ) ) {
                vy += 0.2;
            }
            dy -= (s16)vy;
            if(vy < 0) {
                droid.setCharacter(10);
            } else {
                droid.setCharacter(8);
            }
            if ( dy < 0 ) {
                dy = 0;
                vy = -0.;
            }

            if ( (vy < 0) &&
                 ((ax - 11) < dx) &&
                 (dx < (ax + 11)) ) {
                if ( ay - 13 < dy ) {
                    /* りんごに乗る */
                    dy = ay - 13;
                    state = 0;
                }
            }

            if ( 120 < dy ) {
                /* 着地 */
                dy = 120;
                state = 0;
            }
            droid.setPosition(dx, dy);
            vy = vy - 0.3;
            break;
        }
        droid.draw();
    }
}



