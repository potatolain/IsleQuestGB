#include "title.h"
#include "graphics/title_map.h"
#include "graphics/title_tiles.h"
#include <gb/gb.h>

void do_title() {

	disable_interrupts();
	DISPLAY_OFF;
			
	// This is exceedingly lazy/poor use of ROM space... It's one bank, and this is an example program. Deal with it.
	set_bkg_data(0U, 128U, title_tiles);
	set_bkg_tiles(0U, 0U, 20U, 20U, title_map);
	SHOW_BKG;
	
	DISPLAY_ON;
	enable_interrupts();

	
	waitpad(J_START);
	return;
}