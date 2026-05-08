#include "simple_logger.h"
#include "entity.h"
#include "gfc_input.h"
#include "gfc_shape.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"



const double healthStates[HS_COUNT] = { 0.80, 0.40, 0.20 };


//Spacial Hash Stuff

typedef struct Cell_s
{
	int x;
	int y;
	int entityMax;
	Entity** entityList;
}Cell;

typedef struct CellManager_S
{
	Cell* cellList;
	Uint32 width;
	Uint32 height;
	Uint32 cellSize;
	Uint32 cellMax;
}CellManager;

static CellManager cellManager = { 0 };

typedef struct EntityManager_S
{
	Entity* entityList;
	Uint32 entityMax;
	Uint32 entityPool;
	//Uint8 drawBounds;
	Uint8 paused;
	int playerPoints;
}EntityManager;

static EntityManager entityManager = { 0 };

static Entity* thePlayer = NULL;
static Entity* theBoss = NULL;

static int DEFAULT_POINTS = 0;

void entityManagerClose();
void cellManagerClose();
void addToCell(Entity* thing);
void removeFromCell(Entity* thing);
static int _touchChecks(int c, int d, int e);

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
	entityManager.playerPoints = 0;
	
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
		entityManager.entityList[c]._inUse = 0;
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
		entityManager.entityList[c].points = DEFAULT_POINTS;

		addToCell(&entityManager.entityList[c]);

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
	if (!self)
	{
		slog("Trying to free null entity");
		return;
	}

	if (self->_inUse)
	{
		slog("Trying to free in use entity!");
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

void entityKillAllButPlayer()
{
	int c;
	int id;
	id = getPlayer()->id;

	for (c = 0; c < entityManager.entityMax; c++)
	{
		if (entityManager.entityList[c].id == id)
			continue;
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
		if (self->name && toucher->name)
			slog("%s name is trying to touch %s", self->name, toucher->name);
		else
			slog("Self is trying to touch toucher!");
		self->touch(self, toucher);
		//slog("Self has an touch!");
	}
	else
	{
		slog("Entity has no touch!");
	}

	if (toucher->touch)
	{
		if (self->name && toucher->name)
			slog("%s name is trying to touch %s", self->name, toucher->name);
		else
			slog("Toucher is trying to touch self!");
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
	int c, d, e;
	int posX,posY, index;
	//int touchDistance = 50;

	if (!cellManager.cellMax)
		return;
	


	for (c = 0; c < cellManager.cellMax; c++)
	{
		for (d = 0; d < cellManager.cellList[c].entityMax; d++)
		{
			if (!cellManager.cellList[c].entityList[d])
				continue;

			if (cellManager.cellList[c].entityList[d]->team == TEAM_IGNORE)
				continue;

			if(cellManager.cellList[c].entityList[d]->delay > cellManager.cellList[c].entityList[d]->delayTimer)
				continue;

			if (cellManager.cellList[c].entityList[d]->layer == EL_INVISIBLE)
				continue;

			index = cellManager.cellList[c].entityList[d]->currentIndex;

			posX = index % cellManager.width;
			posY = index / cellManager.width;

			//slog("Position of current Entitity in 1D Index: %i X: %i Y: %i", index, posX, posY);
			

			//Found an entity!
			//Check if that entity is touching any entities in cells + 1 distance around it
			//Assuming a 3x3 grid:
			/*
				1,2,3
				4,5,6
				7,8,9
			*/
			//Current entity is 5, need to look at all 9
			//Compare with other entities in same spot first (5)


			//Slot 5
			for (e = d+1; e < cellManager.cellList[c].entityMax; e++)
			{
				if (!_touchChecks(c, d, e))
					continue;

				slog("entities trying to touch!");
				entityTouch(cellManager.cellList[c].entityList[d], cellManager.cellList[c].entityList[e]);	
			}

			
		}
	}


	/*
	for (c = 0; c < entityManager.entityMax; c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;
		if (entityManager.entityList[c].team == TEAM_IGNORE)
			continue;


		//BITWISE & Using the layer system

		for (d = c + 1; d < entityManager.entityMax; d++)
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





			//slog("Comparing touch %s and %s",entityManager.entityList[c].name, entityManager.entityList[d].name);

			//slog("Touching all entities: ");
			entityTouch(&entityManager.entityList[c], &entityManager.entityList[d]);
		}

	}
	*/

	/*if (entityManager.entityList[c].layer == EL_PLAYER && entityManager.entityList[d].layer == EL_SYMBOLS)
	{
		slog("Trying to touch player and symbols!");
	}

	//This will probably not work in the player leaves Quadrant 1. Currently they are locked to Q1.
	if (getDistance(&entityManager.entityList[c], &entityManager.entityList[d]) > touchDistance)
	{
		//Damn! This shit works well! - using distance formula, a bit too heavy on performance eventually
		//slog("Entities are too far away to touch! %s and %s", entityManager.entityList[c].name, entityManager.entityList[d].name);
		continue;
	}*/
}

/*
	Helper function to run through all the touch checks between cells within the nested loops
	If more cells need to be touched, this is where to put conditions
	Immediate false cases, invis, error layer, etc should be caught earlier!
	returns 0 if no touch. 1 if touching
*/
static int _touchChecks(int c, int d, int e)
{
	if (!cellManager.cellList[c].entityList[e])
		return 0;

	if (!cellManager.cellList[c].entityList[d])
		return 0;

	if (cellManager.cellList[c].entityList[e]->_inUse == 0)
		return 0;

	if (!cellManager.cellList[c].entityList[d]->_inUse)
		return 0;

	if (cellManager.cellList[c].entityList[d]->team == cellManager.cellList[c].entityList[e]->team)
		return 0;

	if (cellManager.cellList[c].entityList[d]->layer == cellManager.cellList[c].entityList[e]->layer)
		return 0;

	if ((cellManager.cellList[c].entityList[d]->layer == EL_ITEM || cellManager.cellList[c].entityList[e]->layer == EL_ITEM) && (cellManager.cellList[c].entityList[d]->layer != EL_PLAYER || cellManager.cellList[c].entityList[e]->layer == EL_PLAYER))
		return 0;

	return 1;
}

void outOfBounds(Entity* self)
{
	if (!self)
		return;

	
	if (!self->role)
		goto jump;
	

	//if(self->role != NULL && self->role == ROLE_BOSS1 || self->role == ROLE_BOSS2 || self->role == ROLE_BOSS3 || self->role == ROLE_PLAYER_BAKER || self->role == ROLE_PLAYER_GAMBLER || self->role == ROLE_PLAYER_GUNNER)
	if(self->role != ROLE_PROJECTILE && self->role != ROLE_BOMB)
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
		jump:
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

void setPlayer(Entity* newPlayer)
{
	thePlayer = newPlayer;
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
	else if (strcmp(role, "ROLE_DODGE") == 0)
		return ROLE_DODGE;
	else if (strcmp(role,"ROLE_RUSH") == 0)
		return ROLE_RUSH;
	else
	{
		slog("Get Role returning Error Role");
		return ROLE_ERROR;


	}

}

void setPausedEntity(int value)
{

	/*if (entityManager.paused == PAUSED)
		entityManager.paused = NOT_PAUSED;
	else
		entityManager.paused = PAUSED;*/
}

int getPlayerPoints()
{
	return entityManager.playerPoints;
}

void setPlayerPoints(int newPoints)
{
	entityManager.playerPoints = newPoints;
}

void addPlayerPoints(int add)
{
	entityManager.playerPoints += add;
}

void subtractPlayerPoints(int sub)
{
	entityManager.playerPoints -= sub;
}

Entity* compareMonsterBossID(int bossID)
{
	int c;

	for (c = 0; c < entityManager.entityMax; c++)
	{
		if (!entityManager.entityList[c]._inUse)
			continue;

		if (entityManager.entityList[c].id == bossID)
			return &entityManager.entityList[c];
	}
	return NULL;
}

const char* getRoleFromInt(int role) 
{
	switch (role)
	{			
		case ROLE_TRASHMOB:
			return "ROLE_TRASHMOB";

		case ROLE_DODGE:
			return "ROLE_DODGE";

		case ROLE_RUSH:
			return "ROLE_RUSH";
			

		case ROLE_BOSS1:
			return "ROLE_BOSS1";
			

		case ROLE_BOSS2:
			return "ROLE_BOSS2";
			

		case ROLE_BOSS3:
			return "ROLE_BOSS3";
			

		case ROLE_PROJECTILE:
			return "ROLE_PROJECTILE";
			
			
		case ROLE_BOMB:
			return "ROLE_BOMB";
			

		case ROLE_PU_RANDOM:
			return "ROLE_PU_RANDOM";


		case ROLE_PU_HP_RECOVERY:
			return "ROLE_PU_HP_RECOVERY";

		case ROLE_PU_INVUL:
			return "ROLE_PU_INVUL";

		case ROLE_PU_BOMB:
			return "ROLE_PU_BOMB";

		case ROLE_PU_SPEED:
			return "ROLE_PU_SPEED";

		case ROLE_PU_FREE_ULT:
			return "ROLE_PU_FREE_ULT";

		case ROLE_PLAYER_GAMBLER:
			return "ROLE_PLAYER_GAMBLER";


		case ROLE_PLAYER_BAKER:
			return "ROLE_PLAYER_BAKER";


		case ROLE_PLAYER_GUNNER:
			return "ROLE_PLAYER_GUNNER";

		default:
			slog("Failed to find the role!");
			return "ROLE_ERROR";
	}
}

const char* getTeamFromInt(int team)
{
	switch (team)
	{
		case TEAM_PLAYER:
			return "TEAM_PLAYER";
		case TEAM_ENEMY:
			return "TEAM_ENEMY";

		case TEAM_IGNORE:
			return "TEAM_IGNORE";
		case TEAM_ITEM:
			return "TEAM_ITEM";

		default:
			slog("Failed to find team!");
			return "TEAM_NONE";
	}
}

int getEntityData(GFC_List* data)
{
	if (!data)
	{
		slog("List to return is not real!");
		return 0;
	}

	int c;
	for (c = 0; c < entityManager.entityMax; c++)
	{
		if (entityManager.entityList[c]._inUse == NULL)
			continue;
		
		gfc_list_append(data,&entityManager.entityList[c]);

	}

	return 1;
}

int initializeCells(int width, int height, int cellSize)
{
	int c;

	if (!width)
		return 0;

	if (!height)
		return 0;

	if (!cellSize)
		return 0;

	if (!entityManager.entityMax)
		return;

	cellManager.cellSize = cellSize;
	cellManager.width = ((width + cellSize - 1) / cellSize);
	cellManager.height = ((height + cellSize - 1) / cellSize);
	cellManager.cellMax = cellManager.width * cellManager.height;

	slog("Cellmanager width: %i Height: %i CellMax: %i", cellManager.width, cellManager.height, cellManager.cellMax);

	cellManager.cellList = gfc_allocate_array(sizeof(Cell), cellManager.cellMax);

	slog("CellManager Size: %i ", cellManager.cellMax);

	for (c = 0; c < cellManager.cellMax; c++)
	{
		cellManager.cellList[c].entityMax = entityManager.entityMax;
		cellManager.cellList[c].entityList = gfc_allocate_array(sizeof(Entity*), entityManager.entityMax);

		if (!cellManager.cellList[c].entityList)
		{
			slog("Failed to allocate a cell's entityList!");
			return 0;
		}
	}

	atexit(cellManagerClose);
	slog("Initalized Cell System");
	return 1;
}

void cellManagerClose()
{
	int c,d;

	if (!cellManager.cellList)
		return;

	for (c = 0; c < cellManager.cellMax; c++)
	{
		if (!cellManager.cellList[c].entityList)
			continue;
		

		for (d = 0; d < cellManager.cellList[c].entityMax; d++)
		{
			//Do this only if the cells should own the entity data!
			//cellManager.cellList[c].entityList[d]->_inUse = 0;
			//entityFree(&cellManager.cellList[c].entityList[d]);	
		}
		free(cellManager.cellList[c].entityList);
		cellManager.cellList[c].entityList = NULL;
	}
	
	free(cellManager.cellList);
	
	memset(&cellManager, 0, sizeof(CellManager));
	slog("Closed Cell System");
}

void addToCell(Entity* thing)
{
	int posX, posY, index, c;

	if (!thing)
	{
		slog("Cannot add a NULL entity to a cell!");
		return;
	}
	else
		;//slog("Added an entity to cell!");

	if (thing->currentIndex != -1)
		removeFromCell(thing);

	//79,79 -> 0,0
	//81,81 -> 1,1 * width 2d-> 1d array

	posX = thing->position.x/cellManager.cellSize;
	posY = thing->position.y/cellManager.cellSize;
	

	if(posX >= cellManager.width || posX < 0)
	{
		slog("Entity Position X is outside the cell's bounds! Cannot add!");
		return;
	}

	if (posY >= cellManager.height || posY < 0)
	{
		slog("Entity Position Y is outside the cell's bounds! Cannot add!");
		return;
	}

	index = posX + posY * cellManager.width;

	for (c = 0; c < cellManager.cellList[index].entityMax; c++)
	{
		
		if (cellManager.cellList[index].entityList[c] == NULL)
		{
			cellManager.cellList[index].entityList[c] = thing;
			thing->previousIndex = thing->currentIndex;
			thing->currentIndex = index;
			//slog("Found the space in cell structure to Add!");
			return;
		}
	}
	slog("Failed to add entity to a cell!");

}

void removeFromCell(Entity* thing)
{
	int index, c;

	if (!thing)
	{
		slog("Trying to remove NULL a cell!");
		return;
	}

	index = thing->currentIndex;

	for (c = 0; c < cellManager.cellList[index].entityMax; c++)
	{
		if (cellManager.cellList[index].entityList[c] == thing)
		{
			cellManager.cellList[index].entityList[c] = NULL;
			thing->previousIndex = -1;
			//slog("Found the entity in cell structure to remove!");
			return;
		}
	}
	slog("Failed to find the entity to remove in a cell!");
}

void displayAllCells()
{
	int c, d;

	if (!cellManager.cellMax)
		return;

	for (c = 0; c < cellManager.cellMax; c++)
	{
		if (!cellManager.cellList[c].entityList)
			continue;


		for (d = 0; d < cellManager.cellList[c].entityMax; d++)
		{
			if (!cellManager.cellList[c].entityList[d])
				continue;
			
			if(!cellManager.cellList[c].entityList[d]->name)
				slog("Inside Cell Manager Cell's list: it has an entity!");
			else
				slog("Inside Cell Manager Cell's list: it has an entity! Name: %s", cellManager.cellList[c].entityList[d]->name);
		}
		
	}
}


void moveTowardsSpot(Entity* mover, Entity* spot)
{
	if (!mover || !spot)
		return;

	if (mover->position.x < spot->position.x)
		mover->velocity.x += mover->topSpeed.x;
	else
		mover->velocity.x -= mover->topSpeed.x;

	if (mover->position.y < spot->position.y)
		mover->velocity.y += mover->topSpeed.y;
	else
		mover->velocity.y -= mover->topSpeed.y;
		
}

//endLine
