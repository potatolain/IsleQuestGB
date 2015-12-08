#include "main.h"
#include "sprites.h"
#include "graphics/world_0_sprites.h"
#include "title.h"
#include "helper.h"
 
#include <gb/gb.h>
#include <rand.h>

UBYTE playerX, playerY, playerXVel, playerYVel, playerWorldPos;
enum PLAYER_DIRECTION playerDirection;
UBYTE* currentMap;
UBYTE* * * currentMapSprites;
UBYTE* tempPointer;
UBYTE playerVelocityLock;
UINT8 btns, oldBtns;
UBYTE buffer[20];
UBYTE temp1, temp2, temp3, temp4, temp5;
UBYTE cycleCounter;
UINT16 temp16, temp16b;
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

void move_sprites() NONBANKED {
	temp1 = cycleCounter % MAX_SPRITES;
	// Skip sprites that are offscreen.
	if (sprites[temp1].type == SPRITE_TYPE_NONE)
		return;

	
	if (sys_time % 60U < MAX_SPRITES) {
		// Direction change!
		temp3 = rand() % 8U;
		if (temp3 > SPRITE_DIRECTION_STOP)
			temp3 = SPRITE_DIRECTION_STOP;
		sprites[temp1].direction = temp3;
	} else {
		temp3 = sprites[temp1].direction;
	}
	// Set some vars to the new x/y pos 
	temp2 = 0U; 
	temp4 = 0U;

	// Temporarily hold velocity diff...
	switch (temp3) {
		case SPRITE_DIRECTION_LEFT: 
			temp2 = 0U-SPRITE_MOVEMENT_INTERVAL;
			break;
		case SPRITE_DIRECTION_RIGHT:
			temp2 = SPRITE_MOVEMENT_INTERVAL;
			break;
		case SPRITE_DIRECTION_UP: 
			temp4 = 0U-SPRITE_MOVEMENT_INTERVAL;
			break;
		case SPRITE_DIRECTION_DOWN:
			temp4 = SPRITE_MOVEMENT_INTERVAL;
			break;
	}
	// Test some nice collisions
	// TODO: I took efficiency and wiped my hindquarters with it here. Should consider doing something not garbage...
	// HACK NOTE: Reusing the previously set get_current_map_x_blahblahblah. Should really consider just keeping it in a var.
	if (temp2 != 0U) {
		// Unsigned, so 0 will wrap over to > WINDOW_X_SIZE
		if (sprites[temp1].x + temp2 > (SPRITE_WIDTH/2U) && sprites[temp1].x + temp2 < WINDOW_X_SIZE &&
				// This could be better... both in terms of efficiency and clarity.
				((temp3 == SPRITE_DIRECTION_LEFT && !test_collision(temp16b, sprites[temp1].x + SPRITE_X_FUDGE + temp2, sprites[temp1].y + temp4 + SPRITE_Y_FUDGE) && 
													 !test_collision(temp16b, sprites[temp1].x + SPRITE_X_FUDGE + temp2, sprites[temp1].y + (SPRITE_HEIGHT + SPRITE_Y_FUDGE) + temp4))
				|| 
				 (temp3 == SPRITE_DIRECTION_RIGHT && !test_collision(temp16b, sprites[temp1].x + SPRITE_X_FUDGE, sprites[temp1].y + (SPRITE_HEIGHT + SPRITE_Y_FUDGE) + temp4) && 
													!test_collision(temp16b, sprites[temp1].x + (SPRITE_X_FUDGE + SPRITE_WIDTH), sprites[temp1].y + (SPRITE_HEIGHT + SPRITE_Y_FUDGE) + temp4)))) {
			// Do nothing. No collision.
		} else {
			// Do nothing. Do NOT update sprites.
			temp2 = 0U;
		}
	}
	
	if (temp4 != 0U) {
		// Unsigned, so 0 will wrap over to > WINDOW_Y_SIZE
		if (sprites[temp1].y + temp4 > PLAYER_HEIGHT && sprites[temp1].y + temp4 < (WINDOW_Y_SIZE - STATUS_BAR_HEIGHT - SPRITE_Y_FUDGE) &&
				// This could be better... both in terms of efficiency and clarity.
				((temp3 == SPRITE_DIRECTION_UP && !test_collision(temp16b, sprites[temp1].x - SPRITE_X_FUDGE + temp2, sprites[temp1].y + temp4 + SPRITE_Y_FUDGE) && 
												    !test_collision(temp16b, sprites[temp1].x + (SPRITE_WIDTH - SPRITE_X_FUDGE) + temp2, sprites[temp1].y + SPRITE_Y_FUDGE + temp4))
				|| 
				 (temp3 == SPRITE_DIRECTION_DOWN && !test_collision(temp16b, sprites[temp1].x + (SPRITE_X_FUDGE + SPRITE_WIDTH) + temp2, sprites[temp1].y + SPRITE_Y_FUDGE + temp4) && 
												  !test_collision(temp16b, sprites[temp1].x + (SPRITE_X_FUDGE + SPRITE_WIDTH) + temp2, sprites[temp1].y + (SPRITE_HEIGHT + SPRITE_Y_FUDGE) + temp4)))) {
			// Do nothing. No collision.
		} else {
			// Do nothing. Do NOT update sprites.
			temp4 = 0U;
		}
	}
	
	// And apply the positions.
	sprites[temp1].x += temp2;
	sprites[temp1].y += temp4;


	sprites[temp1].anim_state = ((sys_time & SPRITE_ANIM_INTERVAL) >> SPRITE_ANIM_SHIFT);

	for (temp2 = 0U; temp2 != 4U; temp2++) {
		move_sprite(WORLD_SPRITE_START + (temp1 << 2U) + temp2, sprites[temp1].x + ((temp2 / 2U) << 3U) + SPRITE_X_FUDGE, sprites[temp1].y + ((temp2 % 2U) << 3U) + SPRITE_Y_FUDGE);
		// Little bit hacky... assumes all sprites have 2 states for now.
		// TODO: 48? Again?
		set_sprite_tile(WORLD_SPRITE_START + (temp1 << 2U) + temp2, 48 + (sprites[temp1].anim_state<<2) + temp2);
	}
}

// Test the collision between the player and any solid objects (walls, etc)
UBYTE test_collision(UINT16 base, UBYTE x, UBYTE y) NONBANKED {
	// NOTE: need to understand why x and y need to be offset like this.
	temp16 = base + (MAP_TILE_ROW_WIDTH * (((UINT16)y>>4U) - 1U)) + (((UINT16)x - SPRITE_X_FUDGE)>>4U);
	
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
	temp16b = temp16;
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
	temp2 = 0U; // Position
	while(temp2 != MAX_SPRITES) {
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
	
	while (temp2 != MAX_SPRITES) {
		// Fill in the rest -- both in actual sprites and in our structs.
		for (n = 0U; n < 4U; n++)
			move_sprite(WORLD_SPRITE_START + (temp2 << 2U) + n, SPRITE_OFFSCREEN_X, SPRITE_OFFSCREEN_Y);
		
		sprites[temp2].type = SPRITE_TYPE_NONE;
		sprites[temp2].x = SPRITE_OFFSCREEN_X;
		sprites[temp2].y = SPRITE_OFFSCREEN_Y;
		sprites[temp2].direction = SPRITE_DIRECTION_STOP;
		temp2++;
	}
		
}

// Get the position of the top left corner of a room on the map.
INT16 get_map_tile_base_position() NONBANKED {
	return ((playerWorldPos / 10U) * (MAP_TILE_ROW_WIDTH*MAP_TILE_ROW_HEIGHT)) + ((playerWorldPos % 10U) * MAP_TILES_ACROSS);
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
	playerVelocityLock = 0;
	cycleCounter = 0U;
	 
	for (no = 0; no != MAX_SPRITES; no++) {
		sprites[no].x = SPRITE_OFFSCREEN_X;
		sprites[no].y = SPRITE_OFFSCREEN_Y;
		sprites[no].type = SPRITE_TYPE_NONE;
		sprites[no].data0 = 0U;
		sprites[no].anim_state = 0U;
	}
	
	SWITCH_ROM_MBC1(ROM_BANK_TITLE);
	do_title();
	
	initrand(sys_time);
	
	disable_interrupts();
	DISPLAY_OFF;
	
	// Initialize the screen. We need our sprites and stuff!
	init_screen();
	
	DISPLAY_ON;
	enable_interrupts();
	oldBtns = btns = 0;
	temp16b = get_map_tile_base_position();

	// IT BEGINS!!
	while(1) {
		oldBtns = btns; // Store the old state of the buttons so we can know whether this is a first press or not.
		btns = joypad();
		
		SWITCH_ROM_MBC1(ROM_BANK_HELPER);
		update_player_velocity();
		if (!playerVelocityLock) {
			test_sprite_collision();
		}
		SWITCH_ROM_MBC1(ROM_BANK_WORLD);

		
		
		
		if (playerXVel != 0) {
			// Unsigned, so 0 will wrap over to > WINDOW_X_SIZE
			if (temp1 > (PLAYER_WIDTH/2U) && temp1 < WINDOW_X_SIZE) {
					// This could be better... both in terms of efficiency and clarity.
					if (playerXVel == -PLAYER_MOVE_DISTANCE) { 
						if (!test_collision(temp16b, temp1 + PLAYER_SPRITE_X_OFFSET, temp2) && !test_collision(temp16b, temp1 + PLAYER_SPRITE_X_OFFSET, temp2 + PLAYER_SPRITE_Y_HEIGHT)) {
							playerX = temp1;
							playerDirection = PLAYER_DIRECTION_LEFT;
						}
					} else { 
						if (!test_collision(temp16b, temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2) && !test_collision(temp16b, temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2 + PLAYER_SPRITE_Y_HEIGHT)) {
							playerX = temp1;
							playerDirection = PLAYER_DIRECTION_RIGHT;
						}
					}
			} else if (temp1 >= WINDOW_X_SIZE) {
				playerX = PLAYER_WIDTH + PLAYER_MOVE_DISTANCE;
				playerWorldPos++;
				update_map();
			} else if (temp1 <= (PLAYER_WIDTH/2U)) {
				playerX = WINDOW_X_SIZE - PLAYER_MOVE_DISTANCE;
				playerWorldPos--;
				update_map();
			}
		}
				
		if (playerYVel != 0) {
			// Unsigned, so 0 will wrap over to > WINDOW_Y_SIZE
			if (temp2 > PLAYER_HEIGHT && temp2 < (WINDOW_Y_SIZE - STATUS_BAR_HEIGHT)) {
					// This could be better... both in terms of efficiency and clarity.
					if (playerYVel == -PLAYER_MOVE_DISTANCE) { 
						if (!test_collision(temp16b, temp1 + PLAYER_SPRITE_X_OFFSET, temp2) && !test_collision(temp16b, temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2)) {
							playerY = temp2;
							playerDirection = PLAYER_DIRECTION_UP;
						}
					} else { 
						if (!test_collision(temp16b, temp1 + PLAYER_SPRITE_X_OFFSET, temp2 + PLAYER_SPRITE_Y_HEIGHT) && !test_collision(temp16b, temp1 + (PLAYER_SPRITE_X_OFFSET + PLAYER_SPRITE_X_WIDTH), temp2 + PLAYER_SPRITE_Y_HEIGHT)) {
							playerY = temp2;
							playerDirection = PLAYER_DIRECTION_DOWN;
						}
					}
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
		SWITCH_ROM_MBC1(ROM_BANK_HELPER);
		animate_player();
		SWITCH_ROM_MBC1(ROM_BANK_WORLD);
		move_sprites();
		
		if (playerVelocityLock)
			playerVelocityLock--;
		
		// I like to vblank. I like. I like to vblank. Make the game run at a sane pace.
		wait_vbl_done();
		cycleCounter++;
	}
}