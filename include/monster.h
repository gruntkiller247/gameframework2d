#ifndef __MONSTER_H_
#define __MONSTER_H_

#include "simple_logger.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "entity.h"

//void monsterManagerInit(Uint32 max);

//void monsterManagerClose();

void monsterThink(Entity* self);

void monsterFree(Entity* self);

void monsterUpdate(Entity* self);

void monsterTouch(Entity* self, Entity* toucher);

Entity* monsterEntityNew(GFC_Vector2D position,int role);

/*
	Recieves a GFC Vector of a position for the monster to look at
*/
void lookAt(Entity* self, GFC_Vector2D position);

#endif