#ifndef SPRITES_H
#define SPRITES_H
#include <gb/gb.h>

#define SPRITE_OFFSCREEN_X 0xEF
#define SPRITE_OFFSCREEN_Y 0xEF
#define SPRITE_MOVEMENT_INTERVAL 3U

#define SPRITE_X_FUDGE 8U // Fudge factor for sprites on the X axis. Need to figure out why this is needed...
#define SPRITE_Y_FUDGE 16U

#define SPRITE_ANIM_INTERVAL 0x32U
#define SPRITE_ANIM_SHIFT 5U
#define MAX_SPRITES 6U
#define SPRITE_HEIGHT 16U
#define SPRITE_WIDTH 16U

enum SPRITE_TYPE {
	SPRITE_TYPE_NONE = 0U,
	SPRITE_TYPE_GEN_ENEMY = 1U
};

enum SPRITE_DIRECTION {
	SPRITE_DIRECTION_DOWN = 0U,
	SPRITE_DIRECTION_RIGHT = 1U, 
	SPRITE_DIRECTION_UP = 2U,
	SPRITE_DIRECTION_LEFT = 3U,
	SPRITE_DIRECTION_STOP = 5U
};

struct SPRITE {
	UBYTE x;
	UBYTE y;
	enum SPRITE_TYPE type;
	UBYTE data0;
	UBYTE anim_state;
	enum SPRITE_DIRECTION direction;
};

#endif