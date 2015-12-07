# Isle Quest GB

## What is this thing?

This is a dumb little Gameboy demo written in gbdk I have been playing around with to get to know the system and its quirks. 

It is not meant to be taken seriously, but is available for learning/extending.

## How do I build it?

Unfortunately, this has a dependency on some graphics I cannot provide in binary form due to licensing issues, so by default you will not be able to compile.

You can get there, but there are a few additional steps.

1. Install gbdk for your platform from http://sourceforge.net/projects/gbdk/files/ into the gbdk/ folder.
2. Get a copy of gbtd from http://www.devrs.com/gb/hmgd/intro.html
3. Using gbtd the graphics/ directory, create a file named "base_tiles.gbr"
4. Set the tile size to 16x16
5. Create tiles as follows: 
  * 0 should be blank
  * 1-4 should be grass tiles
  * 7-10 should combine to form a large tree (tile order: top, left, bottom, right)
  * 11-13 should combine to form a large rock
  * 15-18 should be solid objects
  * The top left of 38 Should be a heart
6. Export the tiles to graphics/base_tiles.z80 using TASM, a label of _base_tiles, From 0-40 
7. Create another new file named "base_sprites.gbr" in graphics/
8. Set the tile size to 16x16 again
9. Create tiles as follows
  * 0: player still, face forward
  * 1-2: Player walk forward
  * 3-5 Facing right
  * 6-8: Facing up
  * 9-11: Facing right
10. Export the sprites to graphics/base_sprites.z80 using TASM, a label of _base_sprites, From 0-16
11. Using gbtd in the graphics/ directory, create a file named title_tiles.gbr With tiles for your game's title
12. Export this to graphics/title_tiles.c using the setting for gbdk and a label of title_tiles
13. Using gbmb in the graphics/ directory, create a file named title_map.gbr containing a map of your game's title.
14. Export this to graphics/title_map.c using the setting for gbdk and a label of title_map
19. Finally, go to the base directory and type "make". 

## Can I try it out without doing all that junk?

Yes! Go to http://cpprograms.net/pc-applications/isle-quest-gb/ to play it online, or download the latest rom.
