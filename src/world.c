
#include "world.h"
#include "gf2d_graphics.h"
#include "simple_logger.h"

/*typedef struct World_S
{
	Sprite* background; //Background image
	Sprite* tileSet; //Sprite containing world's tiles
	Uint8* tileMap; //
	Uint32 tileHeight; //height of tiles
	Uint32 tileWidth; //width of tiles
}World;*/

/*
* @brief allocate a new empty world
* @param NULL on error or a blank world
*/
World* worldNew(Uint32 height, Uint32 width)
{
	World* world;
	world = gfc_allocate_array(sizeof(World), 1);

	if (!world)
	{
		slog("Failed to create a world!");
		return NULL;
	}

	if (!width || !height)
	{
		slog("Cannot make a world with bounds of 0");
		return NULL;
	}

	//All world stuff here

	world->tileSet = gfc_allocate_array(sizeof(Uint32), width * height);
	world->tileHeight = height;
	world->tileWidth = height;
	return world;


}

/*
* @brief free a world allocated
* @param world the world is free
*/
void worldFree(World* world)
{
	if (!world)
		return;

	gf2d_sprite_free(world->background);
	gf2d_sprite_free(world->tileSet);
	free(world->tileLayer);
	free(world->tileMap);
	free(world);
}


/*
* @brief draw a world
* @param world to be drawn
*/
void drawWorld(World* world)
{
	int frame;
	int index;
	Uint32 x;
	Uint32 y;

	if (!world)
		return;

	gf2d_sprite_draw(world->background, gfc_vector2d(0, 0),NULL,NULL, NULL, NULL, NULL, 0);

	if (!world->tileSet)
		return;

	for (int c = 0;c < world->tileHeight;c++)
	{
		for (int d = 0;d < world->tileWidth;d++)
		{
			index = d + (c * world->tileWidth);

			if (world->tileMap[index] == 0)
				continue;

			frame = world->tileMap[index] - 1;
			x = d* world->tileSet->frame_w;
			y = c * world->tileSet->frame_w;

			gf2d_sprite_draw(world->tileSet, gfc_vector2d(x,y), NULL, NULL, NULL, NULL, NULL, frame);
		}
		
	}
}

void worldTileLayerBuild(World* world)
{
	if (!world)
		return;

	int frame;
	int index;
	Uint32 x;
	Uint32 y;

	if (world->tileLayer)
	{
		gf2d_sprite_free(world->tileLayer);
	}
	world->tileLayer = gf2d_sprite_new();

	world->tileLayer->surface = gf2d_graphics_create_surface(world->tileWidth * world->tileSet->frame_w, world->tileHeight * world->tileSet->frame_h);

	if (!world->tileLayer->surface)
	{
		slog("Failed to create tileLayer surface!");
		return;
	}


	for (int c = 0;c < world->tileHeight;c++)
	{
		for (int d = 0;d < world->tileWidth;d++)
		{
			index = d + (c * world->tileWidth);

			if (world->tileMap[index] == 0)
				continue;

			frame = world->tileMap[index] - 1;
			x = d * world->tileSet->frame_w;
			y = c * world->tileSet->frame_w;
			


			gf2d_sprite_draw_to_surface(world->tileSet, gfc_vector2d(x, y), NULL, NULL, frame, world->tileLayer);

		}
	}
	world->tileLayer->texture = SDL_CreateTextureFromSurface(gf2d_graphics_get_render(), world->tileLayer->surface);

	if (!world->tileLayer->texture)
	{
		slog("Failed to convert world tile layer to texture");
		return;
	}


}

