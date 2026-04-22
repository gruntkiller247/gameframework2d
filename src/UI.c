#include "UI.h"

typedef struct
{
	UI* uiList;
	Uint32 uiMax;
	Uint32 uiPool;
	
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

	uiManager.uiMax = max;
	atexit(uiManagerClose);
	slog("Initalized UI System");
}

void uiManagerClose()
{
	if (!uiManager.uiMax)
		return NULL;

	int c;
	for (c = 0; c < uiManager.uiMax; c++)
	{
		uiFree(&uiManager.uiList[c]);
	}

	memset(&uiManager, 0, sizeof(uiManager));
	slog("Closed UI System");
}

UI* uiNew(GFC_Vector2D position, GFC_Rect bounds)
{
	if (!uiManager.uiMax)
		return NULL;

	int c;
	for (c = 0; c < uiManager.uiMax; c++)
	{
		if (uiManager.uiList[c]._inUse)
			continue;

		uiManager.uiList[c]._inUse = 1;
		uiManager.uiList[c].id = ++uiManager.uiPool;

		uiManager.uiList[c].active = -1;
		uiManager.uiList[c].bounds = bounds;
		uiManager.uiList[c].position = position;
		uiManager.uiList[c].scale = 1;

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
		return;

	//slog("Trying to draw the UI!");
	if (!self->sprite)
	{
		slog("Failed to load UI's Sprite!");
		return;
	}

	gf2d_sprite_draw(self->sprite, self->position, &self->scale, NULL, &self->rotation, NULL,NULL, (Uint32)self->frame);
}

void uiDrawAll()
{
	int c;


	if (!uiManager.uiList)
	{
		slog("No uiList in Draw All UI!");
		return;
	}

	for (c = 0; c < uiManager.uiMax; c++)
	{
		if (uiManager.uiList[c].active != 1)
		{
			//slog("Skipping!");
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