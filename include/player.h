#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "simple_logger.h"
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "entity.h"
#include "gfc_shape.h"

Entity* playerEntityNew(GFC_Vector2D position, int role);

void playerThink(Entity* self);

void playerFree(Entity* self);

void playerUpdate(Entity* self);

void playerTouch(Entity* self, Entity* toucher);

void playerUpdate(Entity* self);


//Testing Functions:

void playerShoot(Entity* self, int direction);

void makeBomb(Entity* self);

//Entity* playerGet(Entity* self);


//Player Class Functions:

void playerGunnerShoot(Entity* self, int direction);

void playerGunnerSpecial(Entity* self, int direction);

void playerGunnerUltimate(Entity* self);



void playerBakerShoot(Entity* self,int direction);

void playerBakerSpecial(Entity* self, int direction);

void playerBakerUlt(Entity* self);

void playerGamblerShoot(Entity* self, Uint8 direction);

void playerGamblerSpecial(Entity* self, int direction);

void playerGamblerUlt(Entity* self, int direction);


/*
	Checks if touched thing is a powerup and if it should be used!
*/
void playerPowerUps(Entity* self,Entity* powerup);

//void playerMove(Entity* self, int direction);

void loadPlayer(Entity* self);

#endif