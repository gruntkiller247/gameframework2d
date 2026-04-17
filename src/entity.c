#include "simple_logger.h"
#include "entity.h"
#include "gfc_input.h"
#include "gfc_shape.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"



const double healthStates[HS_COUNT] = { 0.80, 0.40, 0.20 };

typedef struct
{
	Entity* entityList;
	Uint32 entityMax;
	Uint32 entityPool;
	//Uint8 drawBounds;
}EntityManager;



static EntityManager entityManager = { 0 };

static Entity* thePlayer = NULL;
static Entity* theBoss = NULL;

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
		
		entityManager.entityList[c].color = GFC_COLOR_TRANSPARENT;
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

	return NULL;
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
		entityManager.entityList[c]._inUse = 0;
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

	if (self->delayTimer < self->delay)
	{
		slog("Entity %s is on a delay!: ", self->name);
		return;
	}


	if (self->layer == EL_INVISIBLE)
	{
		return;
	}
	else if (!gfc_color_cmp(self->color, GFC_COLOR_TRANSPARENT) && self->sprite && self->_inUse)
	{
		gf2d_sprite_draw(self->sprite, self->position, &self->scale, /*&thing*/NULL, &self->rotation, NULL, &self->color, (Uint32)self->frame);
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

	if (self->delayTimer < self->delay)
	{
		//slog("Entity %s is on a delay!: ", self->name);
		self->delayTimer++;
		return;
	}

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

	if (self->delayTimer < self->delay)
	{
		slog("Entity %s is on a delay!: ", self->name);
		return;
	}

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
	int touchDistance = 50;

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

			if (entityManager.entityList[c].delay > entityManager.entityList[c].delayTimer || entityManager.entityList[d].delay > entityManager.entityList[d].delayTimer)
				continue;

			if (entityManager.entityList[d].team == entityManager.entityList[c].team)
				continue;

			if (entityManager.entityList[d].team == TEAM_IGNORE)
				continue;

			if (entityManager.entityList[c].layer == entityManager.entityList[d].layer)
				continue;

			if (entityManager.entityList[c].layer == EL_ITEM && entityManager.entityList[d].layer != EL_PLAYER)
				continue;

			if (entityManager.entityList[d].layer == EL_ITEM && entityManager.entityList[c].layer != EL_PLAYER)
				continue;

			if (entityManager.entityList[c].layer == EL_INVISIBLE || entityManager.entityList[d].layer == EL_INVISIBLE)
				continue;



			/*if (entityManager.entityList[c].layer == EL_PLAYER && entityManager.entityList[d].layer == EL_SYMBOLS)
			{
				slog("Trying to touch player and symbols!");
			}

			//This will probably not work in the player leaves Quadrant 1. Currently they are locked to Q1.
			if (getDistance(&entityManager.entityList[c], &entityManager.entityList[d]) > touchDistance)
			{
				//Damn! This shit works well!
				//slog("Entities are too far away to touch! %s and %s", entityManager.entityList[c].name, entityManager.entityList[d].name);
				continue;
			}*/
				

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



void outOfBounds(Entity* self)
{
	if (!self)
		return;

	

	if(self->role != NULL && self->role == ROLE_BOSS1 || self->role == ROLE_BOSS2 || self->role == ROLE_BOSS3 || self->role == ROLE_PLAYER_BAKER || self->role == ROLE_PLAYER_GAMBLER || self->role == ROLE_PLAYER_GUNNER)
	{
		//Hard coded size is 1200x720
		//slog("Teleporting Player or Boss!");


		if (self->position.x >= 1100)
		{
			//slog("Entity bounds name %s role is %i",self->name,self->role);
			self->position = gfc_vector2d(1100, self->position.y);
		}
			

		if (self->position.y >= 650)
		{
			//slog("Entity bounds name %s role is %i", self->name, self->role);
			self->position = gfc_vector2d(self->position.x, 650);
		}
			

		if (self->position.x <= -50)
		{
			//slog("Entity bounds name %s role is %i", self->name, self->role);
			self->position = gfc_vector2d(-50, self->position.y);
		}
			

		if (self->position.y <= -50)
		{
			//slog("Entity bounds name %s role is %i", self->name, self->role);
			self->position = gfc_vector2d(self->position.x, -50);
		}
			




	}
	else
	{
		//Neither player not boss
		//slog("NOT PLAYUER Entity bounds name %s role is %i", self->name, self->role);
		if (self->position.x >= 1200)
		{
			//slog("Entity bounds name %s role is %i", self->name, self->role);
			self->_inUse = 0;
		}
			

		if (self->position.y >= 720)
		{
			//slog("Entity bounds name %s role is %i", self->name, self->role);
			self->_inUse = 0;
		}

		if (self->position.x <= -50)
		{
			//slog("Entity bounds name %s role is %i", self->name, self->role);
			self->_inUse = 0;
		}

		if (self->position.y <= -50)
		{
			//slog("Entity bounds name %s role is %i", self->name, self->role);
			self->_inUse = 0;
		}
	}
}

void entityBoundsCheckAll()
{
	int c, d;

	for (c = 0; c < entityManager.entityMax; c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;
		if (entityManager.entityList[c].team == TEAM_IGNORE)
			continue;

		//slog("Checking Bounds");
		outOfBounds(&entityManager.entityList[c]);

	}
}

void setPlayer(Entity* player)
{
	thePlayer = player;
}

Entity* getPlayer()
{
	return thePlayer;
}

void entityImageUpdate(Entity* self, char thing[])
{
	if (!self || !gf2d_sprite_load_all(thing, 128, 128, 16, 0))
		return;

	self->sprite = gf2d_sprite_load_all(thing, 128, 128, 16, 0);
}

void setBoss(Entity* boss)
{
	theBoss = boss;
}


Entity* getBoss() 
{
	return theBoss;
}

int getDistance(Entity* self, Entity* notSelf)
{
	//GFC_Vector2D position;
	int distance;
	//sqrt((x1-x2)^2 + (y1-y2)^2)
	// |x1-x2| + |y1-y2|
	

	distance = abs(self->position.x-notSelf->position.x) + abs(self->position.y - notSelf->position.y);
	//distance = sqrt( pow((self->position.x-notSelf->position.x),2) - pow((self->position.y - notSelf->position.y), 2));
	
	
	/*position.x = self->position.x + distance;
	position.y = self->position.y + distance;*/

	return distance;
}

int getColor(Entity* self, const char* color)
{
	if (!self)
		return;

	if (!color)
	{
		slog("Color is NULL!");
		return 0;
	}
	
	//slog("The inputted color is:%s",color);

	if (strcmp(color, "GFC_COLOR_RED") == 0)
	{
		self->color = GFC_COLOR_RED;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTRED") == 0)
	{
		self->color = GFC_COLOR_LIGHTRED;
	}
	else if (strcmp(color, "GFC_COLOR_DARKRED") == 0)
	{
		self->color = GFC_COLOR_DARKRED;
	}
	else if (strcmp(color, "GFC_COLOR_GREEN") == 0)
	{
		self->color = GFC_COLOR_GREEN;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTGREEN") == 0)
	{
		self->color = GFC_COLOR_LIGHTGREEN;
	}
	else if (strcmp(color, "GFC_COLOR_DARKGREEN") == 0)
	{
		self->color = GFC_COLOR_DARKGREEN;
	}
	else if (strcmp(color, "GFC_COLOR_BLUE") == 0)
	{
		self->color = GFC_COLOR_BLUE;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTBLUE") == 0)
	{
		self->color = GFC_COLOR_LIGHTBLUE;
	}
	else if (strcmp(color, "GFC_COLOR_DARKBLUE") == 0)
	{
		self->color = GFC_COLOR_DARKBLUE;
	}
	else if (strcmp(color, "GFC_COLOR_BLACK") == 0)
	{
		self->color = GFC_COLOR_BLACK;
	}
	else if (strcmp(color, "GFC_COLOR_DARKGREY") == 0)
	{
		self->color = GFC_COLOR_DARKGREY;
	}
	else if (strcmp(color, "GFC_COLOR_GREY") == 0)
	{
		self->color = GFC_COLOR_GREY;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTGREY") == 0)
	{
		self->color = GFC_COLOR_LIGHTGREY;
	}
	else if (strcmp(color,"GFC_COLOR_WHITE") == 0)
	{
		self->color = GFC_COLOR_WHITE;
	}
	else if (strcmp(color, "GFC_COLOR_YELLOW") == 0)
	{
		self->color = GFC_COLOR_YELLOW;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTYELLOW") == 0)
	{
		self->color = GFC_COLOR_LIGHTYELLOW;
	}
	else if (strcmp(color,"GFC_COLOR_DARKYELLOW") == 0)
	{
		self->color = GFC_COLOR_DARKYELLOW;
	}
	else if (strcmp(color, "GFC_COLOR_CYAN") == 0)
	{
		self->color = GFC_COLOR_CYAN;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTCYAN") == 0)
	{
		self->color = GFC_COLOR_LIGHTCYAN;
	}
	else if (strcmp(color, "GFC_COLOR_DARKCYAN") == 0)
	{
		self->color = GFC_COLOR_DARKCYAN;
	}
	else if (strcmp(color, "GFC_COLOR_MAGENTA") == 0)
	{
		self->color = GFC_COLOR_MAGENTA;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTMAGENTA") == 0)
	{
		self->color = GFC_COLOR_LIGHTMAGENTA;
	}
	else if (strcmp(color, "GFC_COLOR_DARKMAGENTA") == 0)
	{
		self->color = GFC_COLOR_DARKMAGENTA;
	}
	else if (strcmp(color, "GFC_COLOR_BROWN") == 0)
	{
		self->color = GFC_COLOR_BROWN;
	}
	else if (strcmp(color, "GFC_COLOR_ORANGE") == 0)
	{
		self->color = GFC_COLOR_ORANGE;
	}
	else if (strcmp(color, "GFC_COLOR_LIGHTORANGE") == 0)
	{
		self->color = GFC_COLOR_LIGHTORANGE;
	}
	else if (strcmp(color, "GFC_COLOR_DARKORANGE") == 0)
	{
		self->color = GFC_COLOR_DARKORANGE;
	}
	else
	{
		slog("Unkown macro color! No color applied!");
		return 0;
	}
	return 1;
}

int getRole(const char* role)
{
	if (!role)
	{
		slog("Role has a bad pointer!");
		return ROLE_ERROR;
	}

	if (strcmp(role, "ROLE_TRASHMOB") == 0)
		return ROLE_TRASHMOB;
	else if (strcmp(role, "ROLE_PROJECTILE") == 0)
		return ROLE_PROJECTILE;
	else if (strcmp(role, "ROLE_BOMB") == 0)
		return ROLE_BOMB;
	else if (strcmp(role, "ROLE_BOSS1") == 0)
		return ROLE_BOSS1;
	else if (strcmp(role, "ROLE_BOSS2") == 0)
		return ROLE_BOSS2;
	else if (strcmp(role, "ROLE_BOSS3") == 0)
		return ROLE_BOSS3;
	else if (strcmp(role, "ROLE_PLAYER_GAMBLER") == 0)
		return ROLE_PLAYER_GAMBLER;
	else if (strcmp(role, "ROLE_PLAYER_BAKER") == 0)
		return ROLE_PLAYER_BAKER;
	else if (strcmp(role, "ROLE_PLAYER_GUNNER") == 0)
		return ROLE_PLAYER_GUNNER;
	else if (strcmp(role, "ROLE_PU_BOMB") == 0)
		return ROLE_PU_BOMB;
	else if (strcmp(role, "ROLE_PU_FREE_ULT") == 0)
		return ROLE_PU_FREE_ULT;
	else if (strcmp(role, "ROLE_PU_HP_RECOVERY") == 0)
		return ROLE_PU_HP_RECOVERY;
	else if (strcmp(role, "ROLE_PU_INVUL") == 0)
		return ROLE_PU_INVUL;
	else if (strcmp(role, "ROLE_PU_SPEED") == 0)
		return ROLE_PU_SPEED;
	else if (strcmp(role, "ROLE_PU_RANDOM") == 0)
		return ROLE_PU_RANDOM;
	else
	{
		slog("Get Role returning Error Role");
		return ROLE_ERROR;


	}

}

//endLine
