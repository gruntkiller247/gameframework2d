#ifndef __ENTITY_H__
#define __ENTITY_H__

#include <SDL.h>
#include <stdio.h>
#include "gfc_text.h"
#include "gf2d_sprite.h"
#include "gfc_shape.h"

#define showSlog if (functionSlogs == 1)
static const functionSlogs = 1;

static int playerPoints = 0;

typedef struct Entity_S
{
	//All Entities Require
	Uint8 _inUse;								//No Touch
	Uint64* id;									//What Number Entity I am
	Uint64* layer;								//What layer I am on

	GFC_TextLine name;							//Name of the entity
	GFC_Vector2D position;						//Coordinates in 2d space
	GFC_Vector2D scale;
	GFC_Rect bounds;							//CHANGE TO CIRCLE AT SOME POINT!
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
	GFC_Color color;

	int role;

	Uint8 team;						//ENUM For what team entity is on
	int hp;
	int maxHP;
	int hitDelay;				   //Delay before the entity can take damage again
	int hitTimer;
	int isInvul;
	int damage;
	
	
	//Stuff that is doubled used by various things - should probably seperate at some point
	int isSpecialBomb;
	int currentPowerUp;
	int powerUpMaxTime;
	int ultPowerup;
	int powerUpTimer;

	//This stuff is for the powerup on the player


	//This stuff is for the entities: Assumed most entities required at least 1 timer. Use these before making more! - Clean up later and seperate maybe

	int timerPrimary;			//Timer that counts up to cooldown
	int primaryCooldown;		//Time until primary attack can be fired
	
	//For the bomb entity 
	int move;					//Indicates whether the bomb should be able to move: 0 no; 1 yes


	int timerDeath;				//Timer to count up to timeToLive
	int timeToLive;				//Time to Live for projectiles like things. Can be NULL; Used for Player projectiles + Bombs

	int ultIs;					//This should only be touched by the projectile class when the gunner ults. Indicates if the projectile is a gunner ult - probably needs a rework

	
	int delay;					//Used to delay an entities spawn: To be used in the level editor
	int delayTimer;				//Used to count up to delayMax: Starts at 0

	
}Entity;

typedef enum
{
	HS_HEALTHY,
	HS_INJURED,
	HS_NEAR_DEATH,
	HS_COUNT

}HealthStatesEnum;

extern const double healthStates[HS_COUNT];


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
	EL_SYMBOLS = 256,
	EL_UI = 512
}EntityLayers;

typedef enum DS
{
	D_NONE = -1,
	D_NORTH = 0,
	D_NORTHEAST,
	D_EAST,
	D_SOUTHEAST,
	D_SOUTH,
	D_SOUTHWEST,
	D_WEST,
	D_NORTHWEST,
	D_MAX

}Directions;


typedef enum RN
{
	ROLE_ERROR = 0,
	ROLE_PLAYER_GUNNER = 1,
	ROLE_PLAYER_BAKER,
	ROLE_PLAYER_GAMBLER,

	ROLE_TRASHMOB,
	ROLE_BOSS1,
	ROLE_BOSS2,
	ROLE_BOSS3,
	ROLE_SYMBOL_ENEMY1,
	ROLE_SYMBOL_ENEMY2,
	ROLE_SYMBOL_ENEMY3,

	ROLE_PROJECTILE,
	ROLE_CUP,
	ROLE_FAKECUP,
	ROLE_SYMBOL1,
	ROLE_SYMBOL2,

	
	ROLE_BOMB,

	ROLE_PU_NONE,
	ROLE_PU_RANDOM,
	ROLE_PU_MIN,		
	ROLE_PU_HP_RECOVERY,
	ROLE_PU_INVUL,
	ROLE_PU_BOMB,
	ROLE_PU_SPEED,
	ROLE_PU_FREE_ULT,
	ROLE_PU_MAX,        

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

/*
	Helper method. Uses the Entity system to check if anything is out of bounds
*/
void entityBoundsCheckAll();


/*
	Returns a pointer to the player or NULL
*/
Entity* getPlayer();

/*
	Sets a global pointer to the player
*/
void setPlayer(Entity* player);

/*
	Updates Entity's image
*/
void entityImageUpdate(Entity* self, char thing[]);

/*
	Sets a global pointer to the boss
*/
void setBoss(Entity* boss);

/*
	Returns Boss pointer. If NULL either no boss or that entity is the Boss
*/
Entity* getBoss();

/*
	Returns an int that is the distance between 2 entity posiitons
	Originally used for Touch Filtering. Deprciated due to bugs Manhatten distance created
*/
int getDistance(Entity* self, Entity* notSelf);

/*
	Recieves an entity and a string representation of a color from a JSON file. Converts the string into a GFC color then applies it to the entity
	Assumes that color is one of the macros in GFC_COLOR!
	DOES NOT COMPARE gfc_color(X,Y,Z) for identity! just the macros!
	Returns 1 if successful, 0 otherwise!
*/
int getColor(Entity* self,const char* color);

/*
	Helper function to parse the JSON to get the correct enum role
*/
int getRole(const char* role);

#endif
