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
#define PLAYER_ANIM_INTERVAL 0x08U // %00000100
#define PLAYER_ANIM_SHIFT 3U

#define HEART_TILE (UBYTE)152

enum PLAYER_DIRECTION {
	PLAYER_DIRECTION_DOWN = 0,
	PLAYER_DIRECTION_RIGHT = 1, 
	PLAYER_DIRECTION_UP = 2,
	PLAYER_DIRECTION_LEFT = 3
} playerDirection;

extern UBYTE base_tiles[]; 
extern UBYTE base_sprites[];
extern UBYTE world_0[];

extern UINT16 sys_time;

// This probably isn't strictly necessary, but I don't like having the order of my function declarations matter. 
// main.c is likely going to become a beast of a file, and for now I'm willing to accept that.
INT16 get_map_tile_base_position();
void update_map();
void init_screen();
UBYTE test_collision(UBYTE x, UBYTE y);
UBYTE animate_player();