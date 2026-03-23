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

/*
	Probably a defunct function that lobs a shot at the player!
	Mainly for testing, might be reused later
*/
void trashShoot(Entity* self,Entity* player);


/*
	Setter method used by the Cup attack that makes the boss visible again
*/
void cupStateUpdate(Entity* self);

/*
	Boss 1 symbols attack. Spawns 2 symbols and changes the boss's color to 1 of them
*/
void symbols(Entity* self);

/*
	returns the current symbol set to explode for Symbol Attack
*/
Uint8 getSymbol(Entity* self);

/*
	Called by the projectile when the symbol is correct!
*/
void correctSymbol(Entity* self);

#endif