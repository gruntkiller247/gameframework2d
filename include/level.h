#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gf2d_sprite.h"
#include "gfc_list.h"
#include "UI.h"
#include "button.h"


typedef struct Level_L
{
	Uint64 width;	//Width of the level
	Uint64 height;	//Height of the level
	//GFC_List* levelMap; //Binary representation of the map
	Sprite* background; //Background image
	Uint8 spawnPowerUps; //Whether or not the level can randomly spawn power ups - on by default - not read by JSON ATM
	const char* name;	//Name of the level
	GFC_List* levelUI;	//List of all UI's in the level!
} Level;

void levelManagerInit(Uint32 max);

void levelManagerClose();


/*
	Creates a level with a height of height
	width of width
	and a default levelMap of 0. This means there are no players. monsters. nor bosses of anykind on the map
	NOT THE CONSTRUCTOR! Helper to make the generic level for the constructor!
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
	   within the level. 
	   Currently makes: Players, Enemies, Bombs, Projectiles, Power ups!
*/
Level* dataLoadLevel(const char* levelName);

/*
	Run by the level editor to save the level in JSON format
*/
void saveLevel();

/*
	Frees all levels
*/
void levelKillAll();
#endif

