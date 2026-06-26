#ifndef __LEVEL_H__
#define __LEVEL_H__

#include "gf2d_sprite.h"
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
	//GFC_List* levelUI;	//List of all UI's in the level!
	Uint8 _inUse;
	int enemiesToKill;

	Uint8 move; //Bool Used to determine if the background of the level moves downward
	Uint8 moveSpeed; //amount of movement the level does downwards per frame
	Sint64 moveCounter; //Counter starts at 0 moves counts up to Max Frame
} Level;

typedef enum
{
	L_MOVE_MIN_FRAME = -720,
	L_MOVE_MAX_FRAME = 0  //(L_MOVE_MIN_FRAME * -1)
}Level_Move;

typedef enum
{
	LS_ERROR = 0,
	LS_NORMAL,
	LS_NEW_LEVEL,
	LS_END_GAME,
	LEVELS_UNTILL_SHOP = 4
}Level_Status;

//void levelManagerInit(Uint32 max);

//void levelManagerClose();

/*
	All this does is ensures that the font is working!
	Must be run before levels can be loaded!
*/
void initalizeLevel();

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

void levelUpdate(const char* levelName);

/*
	Returns the enum status of a level.
	IE when a level needs to be changed, call this to check if the level has changed!
	If the level has changed, then you need to get the new current level
*/
int getLevelStatus();

void setLevelStatus(int status);

Level* getCurrentLevel();

const char* getNextLevel();

/*
	Testing function to test auto inserting the shop level
*/
void setLevelsPlayed(int in);

#endif

