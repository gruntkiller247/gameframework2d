#ifndef __POWERUP_
#define __POWERUP_
#include "entity.h"



/*
	Spawns a powerup at the given coordiants with the specific role, Random role can be given with -1!
*/
Entity* powerUpEntityNew(GFC_Vector2D position, int role);

void powerUpThink(Entity* self);

void powerUpTouch(Entity* self, Entity* toucher);

void powerUpUpdate(Entity* self);

void powerUpFree(Entity* self);
	


#endif