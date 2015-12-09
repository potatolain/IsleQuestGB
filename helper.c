#include "main.h"
#include "helper.h"
#include "sprites.h"

// Animate the player's sprite based on the current system time, if they're moving.
void animate_player()  {
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

// Decrement health by one
void decrease_health() {
	if (health == 0) return;
	hearts[health-1U] = 0x00;
	health--;
	set_win_tiles(2U, 0U, 8U, 1U, hearts);
}

// Increment health by one
void increase_health() {
	if (health > 8) return;
	health++;
	hearts[health-1U] = HEART_TILE;
	set_win_tiles(2U, 0U, 8U, 1U, hearts);

}
void update_player_velocity() {
	if (!playerVelocityLock) {
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
	}
	
	// If this was just pressed for the very first time...
	if (!(oldBtns & J_B) && btns & J_B && health != (UBYTE)0) {
		decrease_health(1);
	}
	
	if (!(oldBtns & J_A) && btns & J_A && health != (UBYTE)8) {
		increase_health(1);
	}
	
	if (!(oldBtns & J_START) && btns & J_START) {
		gameState = GAME_STATE_PAUSE;
		// Darken the screen
		BGP_REG = 0xf9;
		HIDE_SPRITES;
		return;
	}
	
	temp1 = playerX + playerXVel;
	temp2 = playerY + playerYVel;
}

// Did we collide with any sprites?
UBYTE test_sprite_collision() {
	for (temp3 = 0U; temp3 < MAX_SPRITES; temp3++) {
		if (playerX - SPRITE_X_FUDGE < sprites[temp3].x + SPRITE_WIDTH &&
				playerX + (SPRITE_WIDTH - SPRITE_X_FUDGE) > sprites[temp3].x &&
				playerY - SPRITE_Y_FUDGE < sprites[temp3].y + SPRITE_HEIGHT && 
				playerY /*+ SPRITE_HEIGHT*/ > sprites[temp3].y) {// SPRITE_HEIGHT happens to equal our fudge factor (which needs to be sorted out) so... take advantage
			decrease_health();
			playerVelocityLock = 15U;
			playerXVel = 0U-playerXVel;
			playerYVel = 0U-playerYVel;
			return 1;
		}
	}
	return 0;
}

void do_pause_actions() {
	if (!(oldBtns & J_START) && btns & J_START) {
		gameState = GAME_STATE_RUNNING;
		BGP_REG = 0xe4;
		SHOW_SPRITES;
	}
}