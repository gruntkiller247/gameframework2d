#ifndef __BOMB_H__
#define __BOMB_H__

#include "simple_logger.h"
#include "entity.h"

Entity* bombEntityNew(GFC_Vector2D position, Uint8 team, int timeToLive);

//For bomb powerup
Entity* bombEntityNewSpeical(GFC_Vector2D position, Uint8 team, int timeToLive);

void bombThink(Entity* self);

void bombTouch(Entity* self, Entity* toucher);

void bombUpdate(Entity* self);

void bombFree(Entity* self);

/*
	Bomb denotates in 8 directions and spawns projectiles
	Projectiles do Bomb damage/3 damage
*/
void explode(Entity* self);

void bakerExplode(Entity* self);

/*
	A copy of projectile move but for bombs
*/
void moveBomb(Entity* self, int direction);

#endif