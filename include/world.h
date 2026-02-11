#ifndef __WORLD_H__
#define __WORLD_H__

#include "gf2d_sprite.h"


typedef struct World_S
{
	Sprite* background; //Background image
	Sprite* tileSet; //Sprite containing world's tiles
	Sprite* tileLayer; //Prerendered tile layer
	Uint8* tileMap; //
	Uint32 tileHeight; //height of tiles
	Uint32 tileWidth; //width of tiles
}World;

/*
* @brief allocate a new empty world
* @param NULL on error or a blank world
*/
World* worldNew();


/*
* @brief free a world allocated
* @param world the world is free
*/
void worldFree(World* world);


/*
* @brief draw a world 
* @param world to be drawn
*/
void drawWorld(World* world);

#endif