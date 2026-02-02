#include "simple_logger.h"
#include "entity.h"

typedef struct
{
	Entity* entityList;
	Uint32 entityMax;

}EntityManager;

static EntityManager entityManager = { 0 };

void entityManagerClose();

void entityManagerInit(Uint32 max)
{
	if (!max)
	{
		slog("You cannot initalize entity system with 0 entities");
		return;
	}

	entityManager.entityList = gfc_allocate_array(sizeof(Entity),max);

	if (!entityManager.entityList)
	{
		slog("Failed to allocate %i entities", max);
		return;
	}

	entityManager.entityMax = max;
	atexit(entityManagerClose);
	slog("Initalized Entity System");
}

void entityManagerClose()
{
	if (!entityManager.entityMax)
		return NULL;

	int c;
	for (c = 0;c < entityManager.entityMax;c++)
	{
		entityFree(&entityManager.entityList[c]);
	}

	memset(&entityManager, 0, sizeof(EntityManager));
	slog("Closed Entity System");
}


Entity* entityNew()
{
	if (!entityManager.entityMax)
		return NULL;

	int c;
	for (c = 0;c < entityManager.entityMax;c++)
	{
		if (entityManager.entityList[c]._inUse)
			continue;
			
		entityManager.entityList[c]._inUse = 1;
		//set defaults
		entityManager.entityList[c].scale.x = 0;
		entityManager.entityList[c].scale.y = 0;
		return &entityManager.entityList[c];
	}

	return NULL;
}


void entityFree(Entity* self)
{
	if (!self)
		return;

	if (self->sprite)
		gf2d_sprite_free(self->sprite);


	/*if (self->free)
		self->free(self->data);*/


	memset(self, 0, sizeof(Entity));
}

void entityFreeAll()
{
	int c;
	for (c = 0;c < entityManager.entityMax;c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;

		entityFree(&entityManager.entityList[c]);
	}
}

void entityDraw(Entity* self)
{
	if (!self)
	{
		slog("Trying to draw an entity that is NULL!");
		return;
	}

	if (self->sprite)
	{
		gf2d_sprite_draw(self->sprite, self->position, &self->scale, NULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
	}
		

	
}

void entityManagerDrawAll()
{
	int c;
	

	if (!entityManager.entityList)
	{
		return;
	}

	for (c = 0;c < entityManager.entityMax;c++)
	{
		//I think I am supposed to draw the sprites somehow?
		//I think this is correct?
		gf2d_sprite_draw(entityManager.entityList[c].sprite, entityManager.entityList[c].position, &entityManager.entityList[c].scale,NULL,&entityManager.entityList[c].rotation,NULL,NULL,(Uint32)entityManager.entityList[c].frame);
	}
		
	
}

void entityThink(Entity* self)
{
	if (!self)
		return NULL;

	//Thinky
	 
	/*if (self->think())
		self->think(self);*/
}

void entityThinkSystem()
{
	int c;
	for (c = 0;c < entityManager.entityMax;c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;

		entityThink(&entityManager.entityList[c]);
	}
}

void entityUpdate(Entity* self)
{
	if (!self)
		return NULL;

	//Update

	/*if (self->update())
		self->update(self);*/
}

void entityUpdateSystem()
{
	int c;
	for (c = 0;c < entityManager.entityMax;c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;

		entityUpdate(&entityManager.entityList[c]);
	}
}

//endLine
