#include "main.h"
#include "sprites.h"
#include "graphics/world_0_sprites.h"
 
#include <gb/gb.h>

UBYTE playerX, playerY, playerXVel, playerYVel, playerWorldPos;
UBYTE* currentMap;
UBYTE* * * currentMapSprites;
UBYTE* tempPointer;
UINT8 btns, oldBtns;
UBYTE buffer[20];
UBYTE temp1, temp2, temp3;
UINT16 temp16;
// TODO: This feels clumsy... can we re-use buffer for this?
UBYTE hearts[] = {
	HEART_TILE, HEART_TILE, HEART_TILE, HEART_TILE, HEART_TILE, 0U, 0U, 0U
};
UBYTE health;
struct SPRITE sprites[6];

// Initialize the screen... set up all our tiles, sprites, etc.
void init_screen() NONBANKED {
	UBYTE n;
	
	SWITCH_ROM_MBC1(ROM_BANK_TILES);
	update_map();
	
	set_bkg_data(0U, 200U, base_tiles);
	set_win_data(0U, 200U, base_tiles);
	SHOW_BKG;
	
	SPRITES_8x8;
	OBP0_REG = 0x36U; // Set a register to tell it to use our special palette instead of the default one.
	
	SWITCH_ROM_MBC1(ROM_BANK_SPRITES);
	set_sprite_data(0, 100U, base_sprites);
	SHOW_SPRITES;
	 
	move_win(0, 128);
	set_win_tiles(2, 0, 8, 1, hearts);
	SHOW_WIN;
	
	// HEY YOU MAIN CHARACTERS!!
	for (n=0; n<4; n++) {
		set_sprite_tile(n, n);
	}
	
}

// Test the collision between the player and any solid objects (walls, etc)
UBYTE test_collision(UBYTE x, UBYTE y) NONBANKED {
	// NOTE: need to understand why x and y need to be offset like this.
	temp16 = get_map_tile_base_position() + (MAP_TILE_ROW_WIDTH * (((UINT16)y/16U) - 1U)) + (((UINT16)x - SPRITE_X_FUDGE)/ 16U);
	if (currentMap[temp16] > FIRST_SOLID_TILE-1U) {
		return 1;
	}
	return 0;
}

// Update the map when a room change happens. (Or in other cases, I guess)
void update_map() NONBANKED {
	UBYTE n;
	
	
	SWITCH_ROM_MBC1(ROM_BANK_WORLD);
	currentMap = world_0;
	currentMapSprites = world_0_sprites;

	temp16 = get_map_tile_base_position();
	for (n = 0U; n != MAP_TILES_DOWN; n++) {
		for (temp1 = 0U; temp1 != MAP_TILES_ACROSS; temp1++) { // Where 10 = 160/16
			buffer[temp1*2U] = currentMap[temp16 + temp1] * 4U;
			buffer[temp1*2U+1U] = buffer[temp1*2U] + 2U;
		}
		set_bkg_tiles(0U, n*2U, 20U, 1U, buffer);
		
		for (temp1 = 0U; temp1 < 20U; temp1++) { // Where 10 = 160/16
			buffer[temp1]++;
		}
		set_bkg_tiles(0U, n*2U+1U, 20U, 1U, buffer);
		temp16 += MAP_TILE_ROW_WIDTH; // Position of the first tile in this row
	}
	
	tempPointer = currentMapSprites[playerWorldPos];
	temp1 = 0x00; // Generic data
	temp2 = 0; // Position
	while(temp2 != 6U) {
		temp1 = tempPointer++[0];
		if (temp1 == 255U)
			break;
		
		// Temp1 is our position.. convert to x/y
		sprites[temp2].x = (temp1 % 10U) << 4U;
		sprites[temp2].y = (temp1 / 10U) << 4U;

		sprites[temp2].type = tempPointer++[0];
		
		sprites[temp2].data0 = tempPointer++[0];
		// Byte 3 is unused, for now.
		tempPointer++;
		
		// Apply it to some real-world sprites too!
		for (n = 0U; n != 4U; n++) {
			// FIXME: Monster sprites should really be variable...
			set_sprite_tile(WORLD_SPRITE_START + (temp2 << 2U) + n, 48U + n);
			move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + n, sprites[temp2].x + ((n / 2U) << 3U) + SPRITE_X_FUDGE, sprites[temp2].y + ((n % 2U) << 3U) + SPRITE_Y_FUDGE);
		}
		temp2++;
	}
	
	while (temp2 != 6U) {
		// Fill in the rest -- both in actual sprites and in our structs.
		for (n = 0U; n < 4U; n++)
			move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + n, SPRITE_OFFSCREEN_X, SPRITE_OFFSCREEN_Y);
		
		sprites[temp2].x = SPRITE_OFFSCREEN_X;
		sprites[temp2].y = SPRITE_OFFSCREEN_Y;
		temp2++;
	}
		
}

// Get the position of the top left corner of a room on the map.
INT16 get_map_tile_base_position() NONBANKED {
	return ((playerWorldPos / 10U) * (MAP_TILE_ROW_WIDTH*MAP_TILE_ROW_HEIGHT)) + ((playerWorldPos % 10U) * MAP_TILES_ACROSS);
}

// Animate the player's sprite based on the current system time, if they're moving.
void animate_player() NONBANKED {
	temp1 = 0;
	// HACK: We know player sprites start @0, and have 3 per direction in the order of the enum.
	// As such, we can infer a few things based on the direction
	temp1 = (UBYTE)playerDirection * 3U;
	if (playerXVel + playerYVel != 0U) {
		temp1 += ((sys_time & PLAYER_ANIM_INTERVAL) >> PLAYER_ANIM_SHIFT) + 1U;
	}
	
	// Now, shift it left a few times (multiply it by 4 to go from normal sprite to meta-sprite) 
	temp1 = temp1 << 2U;
	for (temp2 = 0U; temp2 != 4U; temp2++) {
		set_sprite_tile(temp2, temp1+temp2);
	}
}

void animate_sprites() NONBANKED  {
	// Little bit hacky... assumes all sprites have 2 states for now.
	// TODO: This 6 is in a few places... MAX_SPRITES?
	for (temp1 = 0; temp1 < 6; temp1++) {
		sprites[temp1].anim_state = ((sys_time & SPRITE_ANIM_INTERVAL) >> SPRITE_ANIM_SHIFT);
		for (temp2 = 0; temp2 < 4; temp2++) {
			// TODO: 48? Again?
			set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U) + temp2, 48 + (sprites[temp1].anim_state<<2) + temp2);
		}
	}
}
 
// Here's our workhorse.
void main(void) {
	UBYTE no;
	
	// Initialize some variables
	playerX = 64;
	playerY = 64;
	health = 5;
	playerWorldPos = 0;
	playerDirection = PLAYER_DIRECTION_DOWN;
	 
	for (no = 0; no != 6; no++) {
		sprites[no].x = SPRITE_OFFSCREEN_X;
		sprites[no].y = SPRITE_OFFSCREEN_Y;
		sprites[no].type = SPRITE_TYPE_NONE;
		sprites[no].data0 = 0U;
		sprites[no].anim_state = 0U;
	}
	 
	disable_interrupts();
	DISPLAY_OFF;
	
	// Initialize the screen. We need our sprites and stuff!
	init_screen();
	
	DISPLAY_ON;
	enable_interrupts();
	oldBtns = btns = 0;
	
	// IT BEGINS!!
	while(1) {
		oldBtns = btns; // Store the old state of the buttons so we can know whether this is a first press or not.
		btns = joypad();
		 
		 playerXVel = playerYVel = 0;

		if (btns & J_UP) {
			playerYVel = -PLAYER_MOVE_DISTANCE;
		}
		
		if (btns & J_DOWN) {
			playerYVel = PLAYER_MOVE_DISTANCE;
		}
		
		if (btns & J_LEFT) {
			playerXVel = -PLAYER_MOVE_DISTANCE;
		}
		
		if (btns & J_RIGHT) {
			playerXVel = PLAYER_MOVE_DISTANCE;
		}
		
		// If this was just pressed for the very first time...
		if (!(oldBtns & J_B) && btns & J_B && health != (UBYTE)0) {
			hearts[health-1U] = 0x00;
			health--;
			set_win_tiles(2U, 0U, 8U, 1U, hearts);
		}
		
		if (!(oldBtns & J_A) && btns & J_A && health != (UBYTE)8) {
			health++;
			hearts[health-1U] = HEART_TILE;
			set_win_tiles(2U, 0U, 8U, 1U, hearts);
		}
		
		temp1 = playerX + playerXVel;
		temp2 = playerY + playerYVel;
		
		if (playerXVel != 0) {
			// Unsigned, so 0 will wrap over to > WINDOW_X_SIZE
			if (temp1 > (PLAYER_WIDTH/2U) && temp1 < WINDOW_X_SIZE &&
					// This could be better... both in terms of efficiency and clarity.
					((playerXVel == -PLAYER_MOVE_DISTANCE && !test_collision(temp1 + PLAYER_SPRITE_X_OFFSET, temp2) && !test_collision(temp1 + PLAYER_SPRITE_X_OFFSET, temp2 + PLAYER_SPRITE_Y_HEIGHT))|| 
					 (playerXVel ==  PLAYER_MOVE_DISTANCE && !test_collision(temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2) && !test_collision(temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2 + PLAYER_SPRITE_Y_HEIGHT)))) {
				playerX = temp1;
				playerDirection = (playerXVel == -PLAYER_MOVE_DISTANCE ? PLAYER_DIRECTION_LEFT : PLAYER_DIRECTION_RIGHT);
			} else if (temp1 >= WINDOW_X_SIZE) {
				playerX = PLAYER_WIDTH + PLAYER_MOVE_DISTANCE;
				playerWorldPos++;
				update_map();
			} else if (temp1 <= PLAYER_WIDTH) {
				playerX = WINDOW_X_SIZE - PLAYER_MOVE_DISTANCE;
				playerWorldPos--;
				update_map();
			}
		}
		
		// See above, unsigned = good!
		if (playerYVel != 0) {
			if (temp2 > PLAYER_HEIGHT && temp2 < (WINDOW_Y_SIZE - STATUS_BAR_HEIGHT) && 
					// See above, again.
					((playerYVel == -PLAYER_MOVE_DISTANCE && !test_collision(temp1 + PLAYER_SPRITE_X_OFFSET, temp2) && !test_collision(temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2))|| 
					 (playerYVel ==  PLAYER_MOVE_DISTANCE && !test_collision(temp1 + PLAYER_SPRITE_X_OFFSET, temp2 + PLAYER_SPRITE_Y_HEIGHT) && !test_collision(temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2 + PLAYER_SPRITE_Y_HEIGHT)))) {
				playerY = temp2;
				playerDirection = (playerYVel == -PLAYER_MOVE_DISTANCE ? PLAYER_DIRECTION_UP : PLAYER_DIRECTION_DOWN);
			} else if (temp2 >= (WINDOW_Y_SIZE - STATUS_BAR_HEIGHT)) {
				playerY = PLAYER_HEIGHT + PLAYER_MOVE_DISTANCE;
				playerWorldPos += 10U;
				update_map();
			} else if (temp2 <= PLAYER_HEIGHT) {
				playerY = (WINDOW_Y_SIZE - STATUS_BAR_HEIGHT) - PLAYER_MOVE_DISTANCE;
				playerWorldPos -= 10U;
				update_map();
			}
		}
		
		for (no=0U; no != 4U; no++) {
			move_sprite(no, playerX + (no/2U)*8U, playerY + (no%2U)*8U);
		}
		
		// Heck with it, just animate every tile.
		animate_player();
		animate_sprites();
		
		// I like to vblank. I like. I like to vblank. Make the game run at a sane pace.
		wait_vbl_done();
	}
}