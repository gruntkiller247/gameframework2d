#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_input.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "projectiles.h"

/*typedef struct
{
	Entity* monsterList;
	Uint32 monsterMax;

}MonsterManager;

static MonsterManager monsterManager = { 0 };

//void monsterManagerClose();

void monsterManagerInit(Uint32 max)
{
	if (!max)
	{
		slog("You cannot initalize monster system with 0 monsters");
		return;
	}

	monsterManager.monsterList = gfc_allocate_array(sizeof(Entity), max);

	if (!monsterManager.monsterList)
	{
		slog("Failed to allocate %i entities", max);
		return;
	}

	monsterManager.monsterMax = max;
	atexit(monsterManagerClose);
	slog("Initalized Monster System");
}*/

typedef enum
{
	MS_IDLE,
	MS_HUNT,
	MS_ATTACK,
	MS_CUP,
	MS_PAIN,
	MS_DIE,
	MS_MAX
}MonsterStates;

typedef enum
{
	MP_TRASH = 0,//Default, should never change if trashmob
	MP_IDLE,	//Default behavior for Boss: if equals this make them randomly select an attack from the stuff below!
	MP_CUP,		//Spawn 3 cups and turn invisible/invul until real cup is killed!
	MP_SYMBOLS, //Spawn 3 symbols(colored projectiles) Player must stand on the one the boss is colored! ->Boss is invul during this!
	MP_AOE,
	MP_SYMBOLS_MOBS,
	MP_MATH,
	MP_MAX
}MonsterPhase;

typedef struct MD
{
	Entity* player;
	MonsterStates state;
	MonsterPhase phase;
	Uint8 phaseCount;

	Uint8 symbolNum;
	Entity* symbol1;
	Entity* symbol2;

	Uint8 aoeTimer;
	Uint8 aoeMaxTime;
	Uint8 aoeSide;

	GFC_Color* symbolMons;
	Uint8 symbolMonsColor;
	Uint8 symbolOrder;		//Abstract int. Increments when the correct color is killed. If it equals the num of minions spawned, determins the phase ending!
}MonsterData;

Entity* theBoss = NULL;

void monsterThink(Entity* self);
void monsterFree(Entity* self);
void monsterUpdate(Entity* self);
void monsterTouch(Entity* self,Entity* toucher);

void trashShoot(Entity* self, Entity* player);
void cupShoot(Entity* self);
void symbols(Entity* self);
void aoe(Entity* self);
void symbolPattern(Entity* self);



Entity* monsterEntityNew(GFC_Vector2D position,int role)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		return NULL;
		slog("Failed to spawn a monster entity!");
	}

	self->sprite = gf2d_sprite_load_all("images/space_bug.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = monsterThink;
	self->update = monsterUpdate;
	self->free = monsterFree;
	self->touch = monsterTouch;

	self->velocity = gfc_vector2d(0, 0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;

	self->bounds = gfc_rect(30, 30, 72, 72);

	self->team = TEAM_ENEMY;
	self->layer = EL_MONSTER;
	
	self->role = role;

	self->hp = 2;
	self->damage = 1;

	self->hitDelay = 300;
	self->hitTimer = 0;
	self->isInvul = 0;

	self->primaryCooldown = 90;
	self->timerPrimary = 0;
	//self->layer = EL_BOSS;

	//self->data = gfc_allocate_array(sizeOf(struct MonsterData), 1);
	//if (data)


	strcpy(self->name, "MONSTER");

	MonsterData* monsterData = malloc(sizeof(MonsterData));
	//MonsterPhase* monsterPhase = malloc(sizeof(MonsterPhase));


	if (!monsterData)
		return NULL;

	/*if (!monsterPhase)
		return NULL;*/

	monsterData->player = getPlayer();
	monsterData->state = MS_IDLE;

	switch (role)
	{
		case ROLE_BOSS1:
			monsterData->phase = MP_IDLE;
			monsterData->phaseCount = 0;
			self->layer = EL_BOSS;

			monsterData->symbol1 = NULL;
			monsterData->symbol2 = NULL;
			setBoss(self);
			break;

		case ROLE_BOSS2:
			self->layer = EL_BOSS;
			monsterData->phase = MP_IDLE;
			monsterData->phaseCount = 0;
			self->basicPlayerProjectileLife = 500;

			monsterData->aoeMaxTime = 10;
			monsterData->aoeTimer = 0;

			monsterData->symbolMons = NULL;
			monsterData->symbolMonsColor = 0;

			setBoss(self);
			break;

		case ROLE_BOSS3:

			self->layer = EL_BOSS;
			setBoss(self);
			break;

		case ROLE_SYMBOL_ENEMY1:
			//self->layer = EL_MONSTER;
			self->colorReal = GFC_COLOR_DARKRED;
			self->layer = EL_MONSTER;
			monsterData->phase = MP_TRASH;
			theBoss = getBoss();

			break;

		case ROLE_SYMBOL_ENEMY2:
			self->colorReal = GFC_COLOR_DARKYELLOW;
			self->layer = EL_MONSTER;
			monsterData->phase = MP_TRASH;
			theBoss = getBoss();
			break;

		case ROLE_SYMBOL_ENEMY3:
			self->colorReal = GFC_COLOR_DARKBLUE;
			self->layer = EL_MONSTER;
			monsterData->phase = MP_TRASH;
			theBoss = getBoss();
			break;

		default: //Trashmod
			monsterData->phase = MP_TRASH;
			self->layer = EL_MONSTER;
			self->timeToLive = 800;
			break;
	}
		

	self->data = monsterData;


	return self;
}


void monsterTouch(Entity* self, Entity* toucher)
{
	if (!self || !toucher)
		return;

	float selfLeft = self->position.x + self->bounds.x;
	float selfRight = selfLeft + self->bounds.w;
	float selfTop = self->position.y + self->bounds.y;
	float selfBottom = selfTop + self->bounds.h;

	float toucherLeft = toucher->position.x + toucher->bounds.x;
	float toucherRight = toucherLeft + toucher->bounds.w;
	float toucherTop = toucher->position.y + toucher->bounds.y;
	float toucherBottom = toucherTop + toucher->bounds.h;

	if (selfLeft < toucherRight && selfRight > toucherLeft && selfTop  < toucherBottom && selfBottom > toucherTop)
	{
		//slog("Monster is touching something!");

		if (toucher->team == TEAM_PLAYER && self->isInvul == 0)
		{
			self->hp -= toucher->damage;
			self->isInvul = 1;
			slog("Player aligned thing touched me %s. HP is now %i",self->name,self->hp);
		}
	}
}

void monsterUpdate(Entity* self)
{
	if (!self)
		return;

	//slog("Updateing Monster!");

	self->frame += 0.1;
	if (self->frame >= 8)
		self->frame = 0;

	//I hate writing code like this but debugging the wall of text made my migraine worse
	float x = self->position.x + self->bounds.x;
	float y = self->position.y + self->bounds.y;
	float w = self->bounds.w;
	float h = self->bounds.h;

	GFC_Vector2D TL = gfc_vector2d(x, y);
	GFC_Vector2D TR = gfc_vector2d(x + w, y);
	GFC_Vector2D BR = gfc_vector2d(x + w, y + h);
	GFC_Vector2D BL = gfc_vector2d(x, y + h);

	gf2d_draw_line(TL, TR, GFC_COLOR_RED);
	gf2d_draw_line(TR, BR, GFC_COLOR_RED);
	gf2d_draw_line(BR, BL, GFC_COLOR_RED);
	gf2d_draw_line(BL, TL, GFC_COLOR_RED);
}

void monsterThink(Entity* self)
{
	//GFC_Vector2D toPlayer = { 0 };
	//MonsterData *data;

	if (!self)
		return;

	if (!((MonsterData*)self->data))
	{
		slog("Monster somehow has no data! Killing it!");
		self->_inUse = 0;
		return;
	}

	if (self->layer != EL_INVISIBLE && self->role == ROLE_BOSS1)
	{
		/*
		int timerPrimary;				//Timer that counts up to cooldown
		int primaryCooldown;			//Time until primary attack can be fired
		nt basicPlayerProjectileLife;  //Projectile timer to live cap for the Player
		*/
		//Boss 1 logic
		

		if (!((MonsterData*)self->data)->player)
		{
			slog("I do not know about the player!");

		}
		else
		{
			//slog("I know about the player!");
			if (self->timerPrimary >= self->primaryCooldown)
			{
				//trashShoot(self, ((MonsterData*)self->data)->player);
				

				if (((MonsterData*)self->data)->phase == MP_IDLE && ((MonsterData*)self->data)->phaseCount == 0)
				{
					slog("Boss is doing Cup Attack!");
					((MonsterData*)self->data)->state = MP_CUP;
					cupShoot(self);

				}
				else if (((MonsterData*)self->data)->phase == MP_IDLE && ((MonsterData*)self->data)->phaseCount == 1)
				{
					slog("Boss is doing Symbol Attack!");
					((MonsterData*)self->data)->state = MP_SYMBOLS;
					symbols(self);
				}
				else
				{
					slog("Boss 1 must be in IDLE state or waiting for a puzzle! phase count is %i", ((MonsterData*)self->data)->phaseCount);

					if (((MonsterData*)self->data)->phase == MP_SYMBOLS || ((MonsterData*)self->data)->phase == MP_CUP)
					{
						self->isInvul = 1;
					}

					if (((MonsterData*)self->data)->phase == MP_IDLE && ((MonsterData*)self->data)->phaseCount >= 2)
					{
						((MonsterData*)self->data)->phaseCount = 0;
					}

				}
				self->timerPrimary = 0;
			}
			else
				self->timerPrimary++;
		}

	}
	else if (self->layer != EL_INVISIBLE && self->role == ROLE_BOSS2)
	{
		//Boss 2 Logic
		if (!((MonsterData*)self->data)->player)
		{
			slog("I do not know about the player!");

		}
		else
		{
			

			if (self->timerPrimary >= self->primaryCooldown)
			{
				//slog("I should shoot!");
				if (((MonsterData*)self->data)->phase == MP_IDLE && ((MonsterData*)self->data)->phaseCount == 0)
				{
					((MonsterData*)self->data)->phaseCount++;
					//slog("AOE!");
					//aoe(self);
				}
				else if (((MonsterData*)self->data)->phase == MP_IDLE && ((MonsterData*)self->data)->phaseCount == 1)
				{
					//((MonsterData*)self->data)->phaseCount++;
					symbolPattern(self);
				}
				else
				{
					
					
					if (((MonsterData*)self->data)->phase == MP_AOE || ((MonsterData*)self->data)->phase == MP_SYMBOLS_MOBS)
					{
						self->isInvul = 1;
					}

					if (((MonsterData*)self->data)->phase == MP_SYMBOLS_MOBS)
					{
						slog("Changing colors!");

						if (((MonsterData*)self->data)->symbolMonsColor > 3)//This check if being hardcoded temporarly!!! Fix this if you want > 3 mobs for this attack!
						{
							((MonsterData*)self->data)->symbolMonsColor = 0;
						}

						if (((MonsterData*)self->data)->symbolMons != NULL)
						{
							self->colorReal = ((MonsterData*)self->data)->symbolMons[((MonsterData*)self->data)->symbolMonsColor];
						}
						
						((MonsterData*)self->data)->symbolMonsColor++;
					}

					if (((MonsterData*)self->data)->phase == MP_IDLE && ((MonsterData*)self->data)->phaseCount >= 2)
					{
						((MonsterData*)self->data)->phaseCount = 0;
					}
				}
				self->timerPrimary = 0;

			}
			else
				self->timerPrimary++;
		}

	}
	else if(self->layer != EL_INVISIBLE)
	{
		if (!((MonsterData*)self->data)->player)
		{
			//slog("I do not know about the player and I am a boss!");
		}
	}
	else
	{
		//slog("I should be invisible!");
	}
	
	//slog("Inside monster thinking. HP is %i", self->hp);
	
	if (self->hp <= 0)
	{
		//slog("I am dead! MR Monster!");
		self->_inUse = 0;
	}

	if (self->isInvul == 1 && self->hitTimer <= self->hitDelay)
	{
		self->hitTimer += 1;
		//slog("Monster is immune, has been for %i", self->hitTimer);
	}
	else
	{
		self->isInvul = 0;
		self->hitTimer = 0;
		//slog("Monster is no longer immune!");
	}

	//slog("Monster is thinking!");

	//I have no fucking clue
	//gfc_vector2d_sub(toPlayer)
}

void monsterFree(Entity* self)
{
	//MonsterData* data;

	if (!self)
		return;

	//slog("Free Monster!");

	
	//clean up anything I own

	if(self->data)
		free(self->data);

	if (self->sprite)
		gf2d_sprite_free(self->sprite);

	/*if (self->color)
		free(self->color);*/

	if(self)
		free(self);


}

/*
	Originally a test function, used for the Boss' Gattling attacks
	Fires a stock projectile at the player entitty
*/
void trashShoot(Entity* self, Entity* player)
{
	if (!self || !player)
		return;

	slog("Trash mob trying to shoot!");



	GFC_Vector2D angle = gfc_vector2d(player->position.x - self->position.x,player->position.y - self->position.y);
	gfc_vector2d_normalize(&angle);

	Entity* projectile = projectileEntityNew(gfc_vector2d(self->position.x + (self->bounds.w/2),self->position.y + (self->bounds.h/2)), TEAM_ENEMY, self->timeToLive, ROLE_PROJECTILE);
	projectile->colorReal = GFC_COLOR_DARKMAGENTA;

	moveProjectileMob(projectile,angle);

}

/*
	Used by Boss 1 to spawn the 3 cups then turn invisible
*/
void cupShoot(Entity* self)
{
	//Idea is spawns 3 cup objects + temporary disables the boss
	//The Boss will be in MS_CUP which means it is not drawn and should not be able to attack
	//The cup projectiles should detect if they are interacted with by the player
	//This function just spawns the cups

	if (!self)
		return;

	self->layer = EL_INVISIBLE;

	int num = rand() % 3;
	Entity* cup1;
	Entity* cup2;
	Entity* cup3;
	int distance = 100;

	

	switch (num)
	{
		case 0:
			cup1 = projectileEntityNew(gfc_vector2d(self->position.x + distance + self->bounds.w,self->position.y), TEAM_ENEMY, -1, ROLE_FAKECUP);
			cup2 = projectileEntityNew(self->position, TEAM_ENEMY, -1, ROLE_FAKECUP);
			cup3 = projectileEntityNew(gfc_vector2d(self->position.x - distance, self->position.y), TEAM_ENEMY, -1, ROLE_CUP);
			break;

		case 1:
			cup1 = projectileEntityNew(gfc_vector2d(self->position.x + distance + self->bounds.w, self->position.y), TEAM_ENEMY, -1, ROLE_FAKECUP);
			cup2 = projectileEntityNew(self->position, TEAM_ENEMY, -1, ROLE_CUP);
			cup3 = projectileEntityNew(gfc_vector2d(self->position.x - distance, self->position.y), TEAM_ENEMY, -1, ROLE_FAKECUP);
			break;

		default:
			cup1 = projectileEntityNew(gfc_vector2d(self->position.x + distance + self->bounds.w, self->position.y), TEAM_ENEMY, -1, ROLE_CUP);
			cup2 = projectileEntityNew(self->position, TEAM_ENEMY, -1, ROLE_FAKECUP);
			cup3 = projectileEntityNew(gfc_vector2d(self->position.x - distance, self->position.y), TEAM_ENEMY, -1, ROLE_FAKECUP);
			break;
	}

	if (!cup1 || !cup2 || !cup3)
	{
		slog("Boss cup attack failed to create all entities!");
		if (cup1)
			cup1->_inUse = 0;

		if (cup2)
			cup2->_inUse = 0;

		if (cup3)
			cup3->_inUse = 0;

		return;
	}

}

/*
	Used by the boss 1's real cup to renable him once the cup dies
*/
void cupStateUpdate(Entity* self)
{
	//Set Boss state to IDLE, might have to check current state

	if (!self)
		return;

	((MonsterData*)self->data)->state=MS_IDLE;
	self->layer = EL_BOSS;
	((MonsterData*)self->data)->phaseCount++; //Temporary to have the boss loop through attacks

	slog("Cup has updated Boss's state! phaseCount ++");
}

void symbols(Entity* self)
{
	int distance = 100;
	int num;
	Entity* sym1;
	Entity* sym2;

	if (!self)
		return;

	slog("Starting Symbol attack!");
	((MonsterData*)self->data)->phase = MP_SYMBOLS;

	/*
		GFC_COLOR_DARKMAGENTA; 
		GFC_COLOR_DARKYELLOW;
	*/

	if (((MonsterData*)self->data)->symbol1)
	{
		slog("Old symbol1, freeing!");
		((MonsterData*)self->data)->symbol1->_inUse = 0;
		return;
	}

	if (((MonsterData*)self->data)->symbol2)
	{
		slog("Old symbol2, freeing!");
		((MonsterData*)self->data)->symbol2->_inUse = 0;
		return;
	}

	slog("Creating new symbols!");
	sym1 = projectileEntityNew(gfc_vector2d(self->position.x - distance, self->position.y), TEAM_ENEMY, -1, ROLE_SYMBOL1);
	sym2 = projectileEntityNew(gfc_vector2d(self->position.x + distance + self->bounds.w, self->position.y), TEAM_ENEMY, -1, ROLE_SYMBOL2);

	if (!sym1 || !sym2)
	{
		slog("Could not create symbols for Boss 1!");

		if (sym1)
			sym1->_inUse = 0;
		if (sym2)
			sym2->_inUse = 0;
	}

	((MonsterData*)self->data)->symbol1 = sym1;
	((MonsterData*)self->data)->symbol2 = sym2;

	num = rand() % 2 + 1;

	((MonsterData*)self->data)->symbolNum = num;

	switch(num)
	{
		case 1:
			self->colorReal = GFC_COLOR_DARKMAGENTA;
			break;
		default:
			self->colorReal = GFC_COLOR_DARKYELLOW;
	}

	if (!((MonsterData*)self->data)->symbol1 || !((MonsterData*)self->data)->symbol2)
	{
		slog("Boss Symbol attack failed to spawn symbols!");

		if (((MonsterData*)self->data)->symbol1)
			((MonsterData*)self->data)->symbol1->_inUse = 0;

		if (((MonsterData*)self->data)->symbol2)
			((MonsterData*)self->data)->symbol2->_inUse = 0;
		return;
	}

}

Uint8 getSymbol(Entity* self)
{
	if (!self)
		return -1;

	return ((MonsterData*)self->data)->symbolNum;
}


void correctSymbol(Entity* self)
{
	if (!self)
		return;

	self->colorReal = GFC_COLOR_TRANSPARENT;

	if (((MonsterData*)self->data)->symbol1 != NULL)
	{
		((MonsterData*)self->data)->symbol1->_inUse = 0;
		((MonsterData*)self->data)->symbol1 = NULL;
	}

	if (((MonsterData*)self->data)->symbol2 != NULL)
	{
		((MonsterData*)self->data)->symbol2->_inUse = 0;
		((MonsterData*)self->data)->symbol2 = NULL;
	}

	((MonsterData*)self->data)->phaseCount++; 
	((MonsterData*)self->data)->phase = MP_IDLE;
	//Tempoary, see cup
}


/*
	Called every frame Boss 2 is running the AOE phase
	Changes the boss' color to indicate whether an attack is coming from the left or right
	Gives the player aoeTimer seconds to prepare, then nukes the field
*/
void aoe(Entity* self)
{
	int num, c;
	GFC_Vector2D pos;
	Entity* thing;

	if (!self)
		return;

	if (((MonsterData*)self->data)->aoeTimer > ((MonsterData*)self->data)->aoeMaxTime)
	{
		//Boss is ready to attack!
		//End this attack
		
	}
	else if (((MonsterData*)self->data)->aoeTimer > 0)
	{
		//Check if the boss has given the player time to think, IE timer > 0
		slog("Current AOE time is: %i", ((MonsterData*)self->data)->aoeTimer);
		((MonsterData*)self->data)->aoeTimer++;
		return;
	}
	else
	{
		//Give the player time to react to the attack + change boss color to either Red or Green

		num = rand() % 2 + 1;
		((MonsterData*)self->data)->aoeSide = num;

		switch (num)
		{
			case 1:
				self->colorReal = GFC_COLOR_RED;
				break;

			default :
				self->colorReal = GFC_COLOR_GREEN;
				break;
		}
		((MonsterData*)self->data)->aoeTimer++;
		return;
	}

	self->colorReal = GFC_COLOR_TRANSPARENT;
	pos = gfc_vector2d(self->position.x,self->position.y);
	((MonsterData*)self->data)->aoeTimer = 0;
	((MonsterData*)self->data)->phaseCount++;
	((MonsterData*)self->data)->phase = MP_IDLE;

	//Teleport the boss to the center of the arena 1200 x 720
	//Then glow either green or red (left - right) 
	//Then enter a waiting period, once the waiting is over, do the actual AOE

	
	switch(((MonsterData*)self->data)->aoeSide)
	{
		case 1:
			//Spawn a ton of projectile moving down to the left!
			//Just going to spawn a bunch of projectiles for 500 units to the left

			for (c = 0; c < 20; c++)
			{
				thing = projectileEntityNew(gfc_vector2d(self->position.x + c*50, self->position.y ), TEAM_ENEMY, self->basicPlayerProjectileLife, ROLE_PROJECTILE);
				moveProjectile(thing, D_SOUTH);
			}
			break;

		default:
			//Spawn a ton of projectile moving down to the right!


			for (c = 0; c < 20; c++)
			{
				thing = projectileEntityNew(gfc_vector2d(self->position.x - c * 50, self->position.y), TEAM_ENEMY, self->basicPlayerProjectileLife, ROLE_PROJECTILE);
				moveProjectile(thing, D_SOUTH);
			}

			break;
	}


}

/*
	Boss 2's symbol attack
	Flashes 3 colors, must kill the monsters in that order, fail and they explode, succeed and the boss is damagble
*/
void symbolPattern(Entity* self)
{
	if (!self)
		return;

	int c,d;
	GFC_Color temp;
	int distance = 100;
	GFC_Color shuffler[3];	//Array of 3 colors to be shuffled then given to the boss to cycle between the colors during this puzzle!
	Entity* mon1;
	Entity* mon2;
	Entity* mon3;

	if (!((MonsterData*)self->data)->symbolMons)
	{
		;
	}
	else
	{
		free(((MonsterData*)self->data)->symbolMons);
		((MonsterData*)self->data)->symbolMons = NULL;
	}

	mon1 = monsterEntityNew(gfc_vector2d(self->position.x - distance, self->position.y), ROLE_SYMBOL_ENEMY1);
	mon2 = monsterEntityNew(gfc_vector2d(self->position.x + distance + self->bounds.w, self->position.y), ROLE_SYMBOL_ENEMY2);
	mon3 = monsterEntityNew(gfc_vector2d(self->position.x + distance * 2 + self->bounds.w, self->position.y), ROLE_SYMBOL_ENEMY3);

	if (!mon1 || !mon2 || !mon3)
	{
		slog("Boss 2 Symbol Monster attack failed!");

		if (mon1)
			mon1->_inUse = 0;

		if (mon2)
			mon2->_inUse = 0;

		if (mon3)
			mon3->_inUse = 0;

		return;
	}

	shuffler[0] = mon1->colorReal;
	shuffler[1] = mon2->colorReal;
	shuffler[2] = mon3->colorReal;
	


	for (c = 0; c < sizeof(shuffler) / sizeof(GFC_Color); c++) 
	{
		
		d = rand() % sizeof(shuffler) / sizeof(GFC_Color);
		temp = shuffler[c];
		shuffler[c] = shuffler[d];
		shuffler[d] = temp;
	}


	if (!(((MonsterData*)self->data)->symbolMons))
	{
		((MonsterData*)self->data)->symbolMons = malloc(sizeof(GFC_Color) * 3);

		if (!((MonsterData*)self->data)->symbolMons)
		{
			slog("error allocating memory to symbolMons!");
			mon1->_inUse = 0;
			mon2->_inUse = 0;
			mon3->_inUse = 0;

			return;
		}
		memcpy(((MonsterData*)self->data)->symbolMons, shuffler, sizeof(GFC_Color) * 3);
	}
	else
	{
		memcpy(((MonsterData*)self->data)->symbolMons, shuffler, sizeof(GFC_Color) * 3);
	}
	
	((MonsterData*)self->data)->phase = MP_SYMBOLS_MOBS;

}

/*
	Helper function called by the ROLE_SYMBOL_ENEMY roles to alert their boss what color they are!
*/
void symbolPatternAlert(Entity* self)
{

}

/*void monsterManagerClose()
{
	if (!monsterManager.monsterMax)
		return NULL;

	int c;
	for (c = 0; c < monsterManager.monsterMax; c++)
	{
		entityFree(&monsterManager.monsterList[c]);
	}

	memset(&monsterManager, 0, sizeof(MonsterManager));
	slog("Closed Entity System");
}*/


