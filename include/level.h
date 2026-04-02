#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gf2d_sprite.h"
#include "gfc_list.h"

typedef struct Level_L
{
	Uint64 width;	//Width of the level
	Uint64 height;	//Height of the level
	GFC_List* levelMap; //Binary representation of the map
	Sprite* background; //Background image
	Uint8 spawnPowerUps; //Whether or not the level can randomly spawn power ups - on by default - not read by JSON ATM
} Level;

/*
	Creates a level with a height of height
	width of width
	and a default levelMap of 0. This means there are no players. monsters. nor bosses of anykind on the map
*/
Level* levelNew(Uint64 height, Uint64 width);

void levelSetBackground(Level* level,Sprite* background);

void levelSetHeight(int maxHeight, int maxWidth);

void levelFree(Level* self);

void levelDraw(Level* self);

/*
	Recieves a filepath to the json level to load
	returns the level and its data
	Creates the entities:
	   Player, Monsters, projectiles, and bombs
*/
Level* dataLoadLevel(const char* levelName);

#endif

