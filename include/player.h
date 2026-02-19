#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "simple_logger.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "gfc_shape.h"

Entity* playerEntityNew(GFC_Vector2D position);

void playerThink(Entity* self);

void playerFree(Entity* self);

void playerUpdate(Entity* self);

void playerTouch(Entity* self, Entity* toucher);

void playerUpdate(Entity* self);

void playerShoot(Entity* self);

//Entity* playerGet(Entity* self);

#endif