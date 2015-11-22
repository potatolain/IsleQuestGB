GRAPHICS_FILES = base_sprites base_tiles 
TILE_FILES = area_0_1

ifeq ($(OS),Windows_NT)
# Nasty little trick to get us a backslash.
	DS := $(shell echo \)
	NULL = nul
else 
	DS = /
	NULL = /dev/null
endif

CC  = gbdk$(DS)bin$(DS)lcc
VBA = tools$(DS)vba$(DS)VisualBoyAdvance
BGB = tools$(DS)bgb$(DS)bgb


# Hack used for the really dirty trickery used to convert maps from gbtd/gbmb later on. Just defines a newline character.
# Note: YES, that is two blank lines on purpose. 
define \n


endef

all:
	@test -d processed || mkdir processed
# Give some useful output if the user didn't grab gbdk yet.	
	@test -d gbdk$(DS)bin || echo "GBDK toolkit not installed in tools/gbdk. Please download and extract it into tools/gbdk."
	
	$(CC) -c -o main.o main.c
# Parse output from gbmd/gbtd into a format asxxxx understands. 
# I'd say this feels dirty, but that would be an insult to dirty things everywhere...
# There are probably at least a dozen cleaner ways to do this, and if you see this and have one, by all means, go for it! I just want a solution that does not require manual changes each time the graphics/tile file is updated.
	$(foreach OBJ,$(GRAPHICS_FILES),echo .globl _$(OBJ) > processed/$(OBJ).s && echo .dw _$(OBJ) >> processed/$(OBJ).s && cat graphics/$(OBJ).z80 | grep -v equ | grep -v end | sed "s/.byte/.db/g" | sed s/\$$/0x/g >> processed/$(OBJ).s${\n})
	$(foreach OBJ,$(TILE_FILES),echo .globl _$(OBJ) > processed/$(OBJ).s && echo .dw _$(OBJ) >> processed/$(OBJ).s && cat graphics/$(OBJ).z80 | grep -v equ | grep -v end | sed "s/.byte/.db/g" | sed s/\$$/0x/g >> processed/$(OBJ).s${\n})
	$(CC) -Wa-l -Wf-bo1 -c -o base_tiles.o processed/base_tiles.s
	$(CC) -Wa-l -Wf-bo2 -c -o base_sprites.o processed/base_sprites.s
	$(CC) -Wa-l -Wf-bo3 -c -o area_0_1.o processed/area_0_1.s
#Duplicate the map for now.
	$(CC) -Wa-l -Wf-bo4 -c -o area_0_1b.o processed/area_0_1.s
	$(CC) -Wl-yt1 -Wl-yo4 -o main.gb main.o area_0_1.o area_0_1b.o base_tiles.o base_sprites.o 


emu: 
	$(BGB) main.gb 
vba:
	$(VBA) main.gb
	
clean:
	-rm -rf processed *.o *.lst *.map *.gb *.rel *.cdb *.ihx *.lnk *.sym *.asm 2> $(NULL)
	
# I use a tool to let me upload the latest binary to my site. If you want to do something similar, create a file named upload(.bat on windows) in tools/
upload: 
	tools$(DS)upload