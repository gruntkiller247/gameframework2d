#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "simple_logger.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "entity.h"

Entity* playerEntityNew(GFC_Vector2D position);

void playerThink(Entity* self);

void playerGet();

#endif