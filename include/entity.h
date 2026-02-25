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
	GFC_Vector2D topSpeed;						//Fastest the entity can move
	GFC_Vector2D velocity;						//The current speed - almost always same as TopSpeed
	void	(*think)(struct Entity_S *self);	//Called every frame if defined for the entity
	void	(*update)(struct Entity_S* self);	
	void	(*free)(struct Entity_S* self);	
	void	(*data) (struct data);
	Uint8	(*touch)(struct Entity_S* self,struct Entity_S* toucher);

	//GFC_Color* color;

	Uint8 team;						//ENUM For what team entity is on
	int hp;
	int hitDelay;				   //Delay before the entity can take damage again
	int hitTimer;
	int isInvul;
	int damage;

	

	//Clean these up and put them in children class at some point
	//ROLE STUFF HERE -First 3 are the Player's stuff
	int role;	//Might be used

	void (*fire)(struct Entity_S* fire,int direction); //Describes how the character attacks
	void (*special)(struct Entity_S* special, int direction); //Describes how the character uses their special attack
	void (*ultimate)(struct Entity_S* ultimate); //Describes how the character uses their ultimate
	
	//This stuff is for the player
	int timerPrimary;				//Timer that counts up to cooldown
	int primaryCooldown;			//Time until primary attack can be fired
	int basicPlayerProjectileLife;  //Projectile timer to live cap for the Player
	
	//Player Baker special values
	int bombTLL;
	int bombAmount;

	int timerDeath;				//Timer to count up to timeToLive
	int timeToLive;				//Time to Live for projectiles like things. Can be NULL; Used for Player projectiles + Bombs

	int timerSpecial;
	int specialCooldown;
	int specialDamage;

	int timerUlt;
	int ultCooldown;
	int ultDamage;
	int ultIs;	//This should only be touched by the projectile class when the gunner ults

	


	
}Entity;


typedef enum DS
{
	D_NORTH,
	D_NORTHEAST,
	D_EAST,
	D_SOUTHEAST,
	D_SOUTH,
	D_SOUTHWEST,
	D_WEST,
	D_NORTHWEST

}Directions;


typedef enum RN
{
	ROLE_PLAYER_GUNNER,
	ROLE_PLAYER_BAKER,
	ROLE_PLAYER_WARRIOR,
	ROLE_TRASHMOB,
	ROLE_COUNT

}RoleNames;


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
