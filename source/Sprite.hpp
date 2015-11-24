/* -*- coding:utf-8; -*- */
#ifndef _SPRITE_H_
#define _SPRITE_H_

#include <gba.h>

namespace GbaGraphics {

const u8 MAX_SPRITES_NUM = 128;

class Sprite {
public:
    enum State {
        gnd, air
    };

    Sprite(u16 n, u16 ch, SPRITE_SIZECODE sizeCode);

    void draw();
    void setHFlip(bool f);
    void setVFlip(bool f);
    void setCharacter(u16 ch);
    void flipH();
    void flipV();
    void setPosition(s16 x, s16 y);
    s16 getPosX() const;
    s16 getPosY() const;

private:
    const u16 num;
    const u16 size;
    const u16 shape;
    u16 character;
    bool hFlip;
    bool vFlip;
    s16 posX;
    s16 posY;

    u16 getSizeBySizeCode(SPRITE_SIZECODE sizeCode) const;
    u16 getShapeBySizeCode(SPRITE_SIZECODE sizeCode) const;
    Sprite(const Sprite&);
    Sprite& operator=(const Sprite&);
    void moveSprite(u16 num, s16 x, s16 y);
    void setSpriteSize(u16 num, u16 size);
    void setSpriteShape(u16 num, u16 shape);
    void setSpriteColor(u16 num, u16 col);
    void setSpriteCharacter(u16 num, u16 ch);
    void setSpriteHFlip(u16 num, bool mode);
    void setSpriteVFlip(u16 num, bool mode);
};


void setSpriteData(const short unsigned int* tiles, int len);
void setSpritePalette(const short unsigned int* sprPal, const int len);
void setBGPalette(const short unsigned int* bgPal, const int len);
void setBGColor(u16 col);
void initSprites();

}

#endif  // _SPRITE_H_
