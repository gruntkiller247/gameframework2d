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
	Uint8* id;									//What Number Entity I am
	Uint32* layer;								//What layer I am on

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
	void	*data;
	void	(*touch)(struct Entity_S* self,struct Entity_S* toucher);

	//GFC_Color* color;
	GFC_Color colorReal;

	Uint8 team;						//ENUM For what team entity is on
	int hp;
	int hitDelay;				   //Delay before the entity can take damage again
	int hitTimer;
	int isInvul;
	int damage;
	
	

	int isSpecialBomb;

	//Clean these up and put them in children class at some point
	//ROLE STUFF HERE -First 3 are the Player's stuff
	int role;	//Might be used

	void (*fire)(struct Entity_S* fire,int direction); //Describes how the entity attacks
	void (*special)(struct Entity_S* special, int direction); //Describes how the entity uses their special attack
	void (*ultimate)(struct Entity_S* ultimate); //Describes how the entity uses their ultimate
	
	
	//This stuff is for the powerup on the player
	int powerUpTimer;
	int powerUpMaxTime;
	int currentPowerUp;
	int ultPowerup;

	//This stuff is for the player
	int timerPrimary;				//Timer that counts up to cooldown
	int primaryCooldown;			//Time until primary attack can be fired
	int basicPlayerProjectileLife;  //Projectile timer to live cap for the Player
	//int amPowered;					//Flag toggled when in a powered up state, only allowed 1 power up at a time!
	
	//Player Baker special values
	int bombTLL;
	int bombAmount;
	int move;

	//Player Gambler specail values
	int ultLength;


	int timerDeath;				//Timer to count up to timeToLive
	int timeToLive;				//Time to Live for projectiles like things. Can be NULL; Used for Player projectiles + Bombs

	int timerSpecial;			//The thing that counts up
	int specialCooldown;		//The thing that is counted to
	int specialDamage;

	int timerUlt;
	int ultCooldown;
	int ultDamage;
	int ultIs;					//This should only be touched by the projectile class when the gunner ults

	


	
}Entity;

typedef enum
{
	PU_NONE = -2,
	PU_RANDOM = -1,
	PU_HP_RECOVERY = 0,
	PU_INVUL,			//Working, uses the hit invul system
	PU_BOMB,			//Spawn a bomb at player's feet
	PU_SPEED,			//Working
	PU_FREE_ULT,			
	PU_MAXNUMBER
}PowerUps;


typedef enum
{
	EL_NONE = 0,
	EL_PLAYER = 1,
	EL_MONSTER = 2,
	EL_BOSS = 4,
	EL_ITEM = 8,
	EL_WORLD = 16,
	EL_PROJECTILES = 32,
	EL_ALL = 36,
	EL_INVISIBLE = 64,
	EL_ENEMY_PROJETILES = 128,
	EL_SYMBOLS = 256
}EntityLayers;

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
	ROLE_PLAYER_GAMBLER,
	ROLE_PLAYER_WARRIOR,//Deprciated
	ROLE_TRASHMOB,
	ROLE_BOSS1,
	ROLE_BOSS2,
	ROLE_BOSS3,
	ROLE_PROJECTILE,
	ROLE_CUP,
	ROLE_FAKECUP,
	ROLE_BOMB,
	ROLE_SYMBOL1,
	ROLE_SYMBOL2,
	ROLE_SYMBOL_ENEMY1,
	ROLE_SYMBOL_ENEMY2,
	ROLE_SYMBOL_ENEMY3,
	ROLE_COUNT

}RoleNames;


typedef enum
{
	TEAM_NONE,
	TEAM_PLAYER,
	TEAM_ENEMY,
	TEAM_IGNORE,
	TEAM_ITEM
}Teams;

//I do not know why this exist, but it should probably be depriciated! - Used by the Player - deprciate Later for the above one
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



//Checks if an Entity is out of bounds, kills it if it is anything except a Player or Boss, otherwise moves them back inbounds
void outOfBounds(Entity* self);

void entityBoundsCheckAll();


/*
	Returns a pointer to the player or NULL
*/
Entity* getPlayer();

void setPlayer(Entity* player);

/*
	Updates Entity's image
*/
void entityImageUpdate(Entity* self, char thing[]);

/*
	Returns a pointer to the boss or NULL
*/
void setBoss(Entity* boss);

/*
	Returns Boss pointer. If NULL either no boss or that entity is the Boss
*/
Entity* getBoss();
#endif
