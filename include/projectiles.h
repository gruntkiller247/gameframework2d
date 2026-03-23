#ifndef __PROJECTILES_H__
#define __PROJECTILES_H__

#include "simple_logger.h"
#include "entity.h"
#include <SDL.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"
#include "gfc_vector.h"


Entity* projectileEntityNew(GFC_Vector2D position, Uint8 team, int* timeToLive, int role);

void projectileThink(Entity* self);

void projectileTouch(Entity* self, Entity* toucher);

void projectileUpdate(Entity* self);

void projectileFree(Entity* self);

void gunnerUlt(Entity* self);


/*
	The direction moves the direction 1 unit based on input
*/
void moveProjectile(Entity* self,int direction);

void moveProjectileMob(Entity* self, GFC_Vector2D position);

void cupExplode(Entity* self);



#endif