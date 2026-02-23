#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include <stdio.h>
#include <time.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"



typedef struct Entity_S
{
	Uint8 _inUse;								//No Touch
	GFC_TextLine name;							//Name of the entity
	GFC_Vector2D position;						//Coordinates in 2d space
	GFC_Vector2D scale;
	GFC_Rect bounds;
	float rotation;
	Sprite* sprite;
	float frame;
	GFC_Vector2D topSpeed;
	GFC_Vector2D velocity;
	void	(*think)(struct Entity_S *self);	//Called every frame if defined for the entity
	void	(*update)(struct Entity_S* self);	
	void	(*free)(struct Entity_S* self);	
	void	(*data) (struct data);
	Uint8	(*touch)(struct Entity_S* self,struct Entity_S* toucher);


	Uint8 team;						//ENUM For what team entity is on
	int hp;
	int hitDelay;				   //Delay before the entity can take damage again
	int hitTimer;
	int isInvul;

	//Clean these up and put them in children class at some point
	//ROLE STUFF HERE -First 3 are the Player's stuff
	void (*fire)(struct Entity_s* fire); //Describes how the character attacks
	void (*special)(struct Entity_s* special); //Describes how the character uses their special attack
	void (*ultimate)(struct Entity_s* ultimate); //Describes how the character uses their ultimate
	
	
	float timerPrimary;				//Timer that counts up to cooldown
	float primaryCooldown;			//Time until primary attack can be fired
	int basicPlayerProjectileLife;  //Projectile timer to live cap for the Player

	int timerDeath;				//Timer to count up to timeToLive
	int timeToLive;				//Time to Live for projectiles like things. Can be NULL;


	
}Entity;


typedef enum
{
	TEAM_NONE,
	TEAM_PLAYER,
	TEAM_ENEMY,

}Teams;

typedef enum
{
	D_UP,
	D_DOWN,
	D_LEFT,
	D_RIGHT,

}Directions;


/*
	@brief initialize the entity sub system
	@param max the upper limit for concurrent entities
*/
void entityManagerInit(Uint32 max);


/*
	@brief get a pointer to a new free entity
	@return Null if out of entities, a pointer to a blank entity otherwise
*/
Entity* entityNew();


/*
	@brief free a previously allocated entity
	@para self the entity to free
	@note do not use the memory address again after calling it
*/
void entityFree(Entity* self);

void entityFreeAll();

void entityDraw(Entity* self);

void entityUpdateAll();

void entityThinkAll();

void entityManagerDrawAll();

void entityManagerClose();

void entityTouch(Entity* self, Entity* toucher);

void entityTouchAll();

/*
	Frees all entitys in the manager regardless of it they're in use
*/
void entityKillAll();

/*
	Sets a pointer value to the player
*/
void playerSetter(Entity* player);

/*
	Returns a pointer value to the player
*/
Entity* playerGetter();


#endif
