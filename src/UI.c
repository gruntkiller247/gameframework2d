#include "UI.h"
#include "button.h"
#include "gf2d_graphics.h"
#include "entity.h"

typedef struct
{
	UI* uiList;
	Uint32 uiMax;
	Uint32 uiPool;
	Uint8 paused;

	Uint8 isSpawning;
	Uint32 roleToSpawn;
}UIManager;

void uiFree(UI* self);

static UIManager uiManager = { 0 };


void uiManagerInit(Uint32 max)
{
	if (!max)
	{
		slog("You cannot initalize UI system with 0 entities");
		return;
	}

	uiManager.uiList = gfc_allocate_array(sizeof(UI), max);

	if (!uiManager.uiList)
	{
		slog("Failed to allocate UI array!");
		return;
	}
	uiManager.isSpawning = 0;
	uiManager.paused = NOT_PAUSED;
	uiManager.uiMax = max;
	atexit(uiManagerClose);
	slog("Initalized UI System");
}

void uiManagerClose()
{
	int c;
	if (!uiManager.uiMax)
		return NULL;

	for (c = 0; c < uiManager.uiMax; c++)
	{
		uiFree(&uiManager.uiList[c]);
	}

	memset(&uiManager, 0, sizeof(uiManager));
	slog("Closed UI System");
}

UI* uiNew(GFC_Vector2D position, GFC_Rect bounds)
{
	int c;
	if (!uiManager.uiMax)
		return NULL;

	for (c = 0; c < uiManager.uiMax; c++)
	{
		if (uiManager.uiList[c]._inUse)
			continue;

		uiManager.uiList[c]._inUse = 1;
		uiManager.uiList[c].id = ++uiManager.uiPool;

		uiManager.uiList[c].active = 0;
		uiManager.uiList[c].bounds = bounds;
		uiManager.uiList[c].position = position;
		uiManager.uiList[c].scale = gfc_vector2d(1,1);

		return &uiManager.uiList[c];
	}
	return NULL;
}

void uiTouch(UI* self)
{
	if (!self)
		return;

	if (self->touch)
		self->touch(self);

	
}

void uiTouchAll()
{
	int c;

	for (c = 0; c < uiManager.uiMax; c++)
	{
		if (!uiManager.uiList[c]._inUse)
			continue;
		
		
		uiTouch(&uiManager.uiList[c]);
	}
}


void uiUpdate(UI* self)
{
	if (!self)
		return;

	if(self->update)
		self->update(self);
}

void uiUpdateAll()
{
	int c;

	for (c = 0; c < uiManager.uiMax; c++)
	{
		if (!uiManager.uiList[c]._inUse)
			continue;
		
		uiUpdate(&uiManager.uiList[c]);
	}
}

void uiDraw(UI* self)
{
	if (!self)
	{
		slog("UI is NULL WHEN DRAWING!");
		return;
	}

	//slog("Trying to draw the UI!");
	if (!self->sprite)
	{
		if (!getButtonTexture(self))
		{
			slog("Failed to load UI's Sprite/Texture to draw!");
			return;
		}

	}

	SDL_Texture* tempTex = NULL;

	tempTex = getButtonTexture(self);

	if (tempTex)
	{

		if (getButtonTexture(self))
		{
			SDL_Rect dest = {
				self->position.x,
				self->position.y,
				self->bounds.w,
				self->bounds.h
			};
			//slog("Trying to draw TEXT UI!");
			SDL_RenderCopy(gf2d_graphics_get_renderer(), getButtonTexture(self), NULL, &dest);

		}
		else
			;
		
	}
	

	//slog("UI Draw Pos: %f %f", self->position.x, self->position.y);



	//SDL_RenderCopy(gf2d_graphics_get_renderer(), texture, NULL, &dstRect);
	

	if (self->hover != 0)
	{
		//slog("Hover not 0!");
		if (self->onHoverSprite)
		{
			//slog("Drawing on hover sprite for UI!");
			gf2d_sprite_draw(self->onHoverSprite, self->position, &self->scale, NULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
		}
		else
		{
			//slog("No on hover sprite for UI element!");
			if (self->sprite)
				gf2d_sprite_draw(self->sprite, self->position, &self->scale, NULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
		}
	}
	else if (self->clicked != 0)
	{
		if (self->onClickSprite)
		{
			//Draw clicked sprite
			gf2d_sprite_draw(self->onClickSprite, self->position, &self->scale, NULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
		}
		else
		{
			//Draw as normal
			if (self->sprite)
				gf2d_sprite_draw(self->sprite, self->position, &self->scale, NULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
		}
	}
	else
	{
		if(self->sprite)
			gf2d_sprite_draw(self->sprite, self->position, &self->scale, NULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
	}
		
}

void uiDrawAll()
{
	int c;


	if (!uiManager.uiList)
	{
		slog("No uiList in Draw All UI!");
		return;
	}
	//slog("Size of UI List: %i", uiManager.uiMax);

	for (c = 0; c < uiManager.uiMax; c++)
	{
		//slog("Active: %i",uiManager.uiList[c].active);
		if (!uiManager.uiList[c]._inUse)
			continue;

		if (!uiManager.uiList[c].active || uiManager.uiList[c].active == 0)
		{
			//slog("Skipping!");
			//slog("Skipping! Unique ID is: %i", uiManager.uiList[c].id);
			continue;
		}
		//slog("Drawing!");

		if (uiManager.uiList[c].activeOnPause == 1 && uiManager.paused != PAUSED)
		{
			continue;
		}

		uiDraw(&uiManager.uiList[c]);
	}
}

UI* uiGetID(Uint32 id)
{
	if (!uiManager.uiList)
		return NULL;

	for (int i = 0; i < uiManager.uiMax; i++)
	{
		if (!uiManager.uiList[i]._inUse)
			continue;

		if (uiManager.uiList[i].id == id)
			return &uiManager.uiList[i];
	}

	return NULL;
}

void uiFree(UI* self)
{
	if (!self)
		return;

	if (self->free)
		self->free(self);

	memset(self, 0, sizeof(UI));

	//free(self);
}

void uiFreeAll()
{
	int c;


	if (!uiManager.uiList)
	{
		return;
	}

	for (c = 0; c < uiManager.uiMax; c++)
	{
		if (uiManager.uiList[c]._inUse == 1)
			continue;
		

		uiFree(&uiManager.uiList[c]);
	}
}

void setPausedUI()
{
	slog("Before: %i", isPaused());

	if (uiManager.paused == PAUSED)
		uiManager.paused = NOT_PAUSED;
	else
		uiManager.paused = PAUSED;
	
	slog("After: %i", isPaused());
}

void uiKillAll()
{
	int c;

	if (!uiManager.uiList)
	{
		return;
	}

	for (c = 0; c < uiManager.uiMax; c++)
	{
		uiFree(&uiManager.uiList[c]);
	}
	
}

int isPaused()
{
	return uiManager.paused;
}

/*
	Recieves a Role from a button to spawn in
	Sets the flag that we are spawning!
*/

void setUISpawning(Uint32 in)
{
	uiManager.isSpawning = S_IS;

	slog("SetUISpawning In: %i", in);

	uiManager.roleToSpawn = in;
	
}

int isSpawning()
{
	return uiManager.isSpawning;
}

int roleToSpawn()
{
	return uiManager.roleToSpawn;
}

void clearSpawning()
{
	uiManager.roleToSpawn = -1;
	uiManager.isSpawning = S_NOT;
	slog("Cleared Spawning!");
}
