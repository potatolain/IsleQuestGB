#ifndef SPRITES_H
#define SPRITES_H
#include <gb/gb.h>

#define SPRITE_OFFSCREEN_X 0xEF
#define SPRITE_OFFSCREEN_Y 0xEF

enum SPRITE_TYPE {
	SPRITE_TYPE_NONE = 0,
	SPRITE_TYPE_GEN_ENEMY = 1
};

struct SPRITE {
	UBYTE x;
	UBYTE y;
	enum SPRITE_TYPE type;
	UBYTE data0;
	UBYTE anim_state;
};

#endif