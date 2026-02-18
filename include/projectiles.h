#ifndef __PROJECTILES_H__
#define __PROJECTILES_H__

#include "simple_logger.h"
#include "entity.h"
#include <SDL.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"


Entity* projectileEntityNew(GFC_Vector2D position, float direction, Uint8 team);

void projectileThink(Entity* self);

void projectileTouch(Entity* self, Entity* toucher);

void projectileUpdate(Entity* self);

void projectileFree(Entity* self);

/*
	The direction the projectile is moving in
*/
void move(Entity* self);

#endif