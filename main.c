#include <gb/gb.h>
#include <gb/drawing.h>
// TODO: This thing needs a header file soon...

#define PLAYER_MOVE_DISTANCE 2

#define WINDOW_X_SIZE (UBYTE)160
#define WINDOW_Y_SIZE (UBYTE)144
#define STATUS_BAR_HEIGHT (UBYTE)20
#define PLAYER_HEIGHT (UBYTE)16U
#define PLAYER_WIDTH (UBYTE)16U
#define MAP_TILES_ACROSS (UBYTE)10U
#define MAP_TILES_DOWN (UBYTE) 8U
#define MAP_TILE_ROW_WIDTH 100U
#define MAP_TILE_ROW_HEIGHT 8U 

#define PLAYER_SPRITE_X_OFFSET 1U
#define PLAYER_SPRITE_X_WIDTH 12U
#define PLAYER_SPRITE_Y_HEIGHT 15U

#define FIRST_SOLID_TILE (UBYTE)6

// Banks
#define ROM_BANK_TILES 1U
#define ROM_BANK_SPRITES 2U
#define ROM_BANK_WORLD 3U

#define HEART_TILE (UBYTE)152

extern UBYTE base_tiles[]; 
extern UBYTE base_sprites[];
extern UBYTE world_0[];
 
UBYTE playerX, playerY, playerXVel, playerYVel, playerWorldPos;
UBYTE* currentMap;
UINT8 btns, oldBtns;
UBYTE buffer[20];
UBYTE temp1, temp2, temp3;
UINT16 temp16;
// TODO: This feels clumsy... can we re-use buffer for this?
UBYTE hearts[] = {
	HEART_TILE, HEART_TILE, HEART_TILE, HEART_TILE, HEART_TILE, 0U, 0U, 0U
};
UBYTE health;
 
INT16 get_map_tile_base_position() {
	return ((playerWorldPos / 10U) * (MAP_TILE_ROW_WIDTH*MAP_TILE_ROW_HEIGHT)) + ((playerWorldPos % 10U) * MAP_TILES_ACROSS);
}
 
void update_map() NONBANKED {
	UBYTE n;
	
	currentMap = world_0;
	
	SWITCH_ROM_MBC1(ROM_BANK_WORLD);
	temp16 = get_map_tile_base_position();
	for (n = 0U; n < MAP_TILES_DOWN; n++) {
		for (temp1 = 0U; temp1 < MAP_TILES_ACROSS; temp1++) { // Where 10 = 160/16
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
}
 
void init_screen() NONBANKED {
	UBYTE n;
	
	update_map();
	
	SWITCH_ROM_MBC1(ROM_BANK_TILES);
	set_bkg_data(0U, 200U, base_tiles);
	set_win_data(0U, 200U, base_tiles);
	SHOW_BKG;
	
	SPRITES_8x8;
	OBP0_REG = 0x36U; // Set a register to tell it to use our special palette instead of the default one.
	
	SWITCH_ROM_MBC1(ROM_BANK_SPRITES);
	set_sprite_data( 0, 100U, base_sprites );
	SHOW_SPRITES;
	 
	move_win(0, 128);
	set_win_tiles(2, 0, 8, 1, hearts);
	SHOW_WIN;
	 
	for (n=0; n<4; n++) {
		set_sprite_tile(n, n);
	}
}

UBYTE test_collision(UBYTE x, UBYTE y) NONBANKED {
	// NOTE: need to understand why x and y need to be offset like this.
	temp16 = get_map_tile_base_position() + (MAP_TILE_ROW_WIDTH * (((UINT16)y/16U) - 1U)) + (((UINT16)x - 8U)/ 16U);
	if (currentMap[temp16] > FIRST_SOLID_TILE-1U) {
		return 1;
	}
	return 0;
}
 
void main(void) {
	UBYTE no;
	 
	playerX = 64;
	playerY = 64;
	health = 5;
	playerWorldPos = 0;
	 
	disable_interrupts();
	DISPLAY_OFF;
 
	init_screen();
	 
	DISPLAY_ON;
	enable_interrupts();
	oldBtns = btns = 0;
	 
	while(1) {
		oldBtns = btns;
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
		
		for (no=0U; no<4U; no++) {
			move_sprite(no, playerX + (no/2U)*8U, playerY + (no%2U)*8U);
		}
		
		// I like to vblank. I like. I like to vblank. Make the game run at a sane pace.
		wait_vbl_done();
	}
}