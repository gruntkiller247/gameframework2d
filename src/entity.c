#include "simple_logger.h"
#include "entity.h"
#include "gfc_input.h"
#include "gfc_shape.h"
#include "gf2d_draw.h"

#include "gf2d_graphics.h"


typedef struct
{
	Entity* entityList;
	Uint32 entityMax;
	Uint32 entityPool;
	//Uint8 drawBounds;
}EntityManager;



static EntityManager entityManager = { 0 };

static Entity* thePlayer = NULL;

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
		entityManager.entityList[c].id = ++entityManager.entityPool;
		//set defaults
		
		entityManager.entityList[c].colorReal = GFC_COLOR_TRANSPARENT;
		entityManager.entityList[c].scale.x = 1;
		entityManager.entityList[c].scale.y = 1;

		return &entityManager.entityList[c];
	}

	return NULL;
}

Entity* entityGetID(Uint32 id)
{
	if (!entityManager.entityList)
		return NULL;

	for (int i = 0; i < entityManager.entityMax; i++)
	{
		if (!entityManager.entityList[i]._inUse)
			continue;

		if (entityManager.entityList[i].id == id)
			return &entityManager.entityList[i];
	}
}


void entityFree(Entity* self)
{
	if (!self || self->_inUse)
	{
		slog("Trying to free null or inUse entity");
		return;
	}

	//if (!self->free)
		//slog("entity, %s , has no free!", self->name);
	//slog("Trying to free %s", self->name);

	/*if (self->data)
		self->free(self->data);
	
	if (self->free)
		self->free(self);
		*/

	
	

	memset(self, 0, sizeof(Entity));
}

void entityFreeAll()
{
	int c;
	for (c = 0;c < entityManager.entityMax;c++)
	{
		
		if (entityManager.entityList[c]._inUse || entityManager.entityList[c]._inUse == NULL)
			continue;


		entityFree(&entityManager.entityList[c]);
		
	}
}


/*
* Remeber, Frees all entities, not set hp to 0!
*/
void entityKillAll()
{
	int c;

	for (c = 0; c < entityManager.entityMax; c++)
	{
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


	
	if (!gfc_color_cmp(self->colorReal, GFC_COLOR_TRANSPARENT) && self->sprite && self->_inUse)
	{
		gf2d_sprite_draw(self->sprite, self->position, &self->scale, /*&thing*/NULL, &self->rotation, NULL, &self->colorReal, (Uint32)self->frame);
	}
	else if (self->sprite && self->_inUse)
	{
		gf2d_sprite_draw(self->sprite, self->position, &self->scale, /*&thing*/NULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
	}
	else
		;
	

	/*if (!self->color && self->sprite && self->_inUse)
	{
		//GFC_Vector2D thing = gfc_vector2d(self->bounds.x / 2, self->bounds.y);

		gf2d_sprite_draw(self->sprite, self->position, &self->scale, /*&thingNULL, &self->rotation, NULL, NULL, (Uint32)self->frame);
		//
		//
		//gf2d_draw_rect(self->bounds, GFC_COLOR_RED); IDK if this is working
	}
	else if (self->sprite && self->_inUse)
	{
		gf2d_sprite_draw(self->sprite, self->position, &self->scale, /*&thingNULL, &self->rotation, NULL, self->color, (Uint32)self->frame);
	}
	else
		;*/



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

		entityDraw(&entityManager.entityList[c]);
		//gf2d_sprite_draw(entityManager.entityList[c].sprite, entityManager.entityList[c].position, &entityManager.entityList[c].scale,NULL,&entityManager.entityList[c].rotation,NULL,NULL,(Uint32)entityManager.entityList[c].frame);
	}


}

void entityThink(Entity* self)
{
	if (!self)
		return;

	/**if (self->name && strcmp(self->name, "Matt") != 0 && strcmp(self->name, "") != 0)
	{
		slog("Making %s Think!", self->name);
	}*/
		

	//Tricking rocks into thinking!
	//slog("Inside thinking. %s is thinking!",self->name);
	self->think(self);


}

void entityThinkAll()
{
	int c;

	if (!entityManager.entityList)
	{
		return;
	}

	for (c = 0;c < entityManager.entityMax;c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;

		if (!entityManager.entityList[c].think)
			continue;

		entityThink(&entityManager.entityList[c]);
	}
}

void entityUpdate(Entity* self)
{
	if (!self)
		return;


	if (self->update)
	{
		self->update(self);
		//slog("Self has an update!");
	}
		
	else
		slog("Entity has no update!");

	
	


}

void entityUpdateAll()
{
	int c;

	for (c = 0;c < entityManager.entityMax;c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;

		//slog("Updating an entity: ");
		entityUpdate(&entityManager.entityList[c]);
	}
}


//IDK If these are needed?
void entityTouch(Entity* self, Entity* toucher)
{
	if (!self || !toucher)
		return;

	if (self->touch)
	{
		self->touch(self, toucher);
		//slog("Self has an touch!");
	}
	else
	{
		slog("Entity has no touch!");
	}

	if (toucher->touch)
	{
		toucher->touch(toucher,self);
	}
	else
	{
		slog("Entity has no touch!");
	}

	
	//slog("Entity has no touch!");
}


void entityTouchAll()
{
	int c,d;

	for (c = 0; c < entityManager.entityMax; c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;
		if (entityManager.entityList[c].team == TEAM_IGNORE)
			continue;
		
		//BITWISE & Using the layer system

		for (d = c+1; d < entityManager.entityMax; d++)
		{
			if (!entityManager.entityList[d]._inUse)
				continue;

			if (entityManager.entityList[d].team == entityManager.entityList[c].team)
				continue;

			if (entityManager.entityList[d].team == TEAM_IGNORE)
				continue;

			if (entityManager.entityList[c].layer == entityManager.entityList[d].layer)
				continue;

			if (entityManager.entityList[c].layer == EL_ITEM && entityManager.entityList[d].layer != EL_PLAYER)
				continue;

			//slog("Comparing touch %s and %s",entityManager.entityList[c].name, entityManager.entityList[d].name);

			//slog("Touching all entities: ");
			entityTouch(&entityManager.entityList[c], &entityManager.entityList[d]);
		}
		
	}
}

void playerSetter(Entity* player)
{
	thePlayer = player;
}

Entity* playerGetter()
{
	return thePlayer;
}

//endLine
