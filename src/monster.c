#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_input.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "projectiles.h"
#include "bomb.h"

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
	MP_HEALTHY_ONCE,
	MP_HEALTHY,
	MP_INJURED_ONCE,
	MP_INJURED,
	MP_NEAR_DEATH_ONCE,
	MP_NEAR_DEATH

}MonsterPhase;

typedef enum
{
	MS_TEST = -1,
	MS_TRASH = 0,//Default, should never change if trashmob
	MS_IDLE,	//Default behavior for Boss: if equals this make them randomly select an attack from the stuff below!
	MS_CUP,		//Spawn 3 cups and turn invisible/invul until real cup is killed!
	MS_SYMBOLS, //Spawn 3 symbols(colored projectiles) Player must stand on the one the boss is colored! ->Boss is invul during this!
	MS_AOE,
	MS_SYMBOLS_MOBS,
	MS_MATH,
	MS_ATTACK,
	MS_DEAD,
	MS_PUZZLE_WAIT,
	MS_PUZZLE1,		//Every Boss has 2 puzzles
	MS_PUZZLE2,
	MS_MAX
}MonsterStates;

typedef enum
{
	B3_NOTHING,
	B3_SNIPE,
	B3_NUKE
}Boss3_Attack;

typedef struct MD
{
	Entity* player;
	MonsterStates state;
	MonsterPhase phase;
	Uint8 phaseCount;	//Does not keep track of the current phase, that is kept track by Monster Phases and checked in Think
	void (*puzzle1)(Entity* self);
	void (*puzzle2)(Entity* self);

	Uint8 symbolNum;
	Entity* symbol1;
	Entity* symbol2;

	Uint8 aoeTimer;
	Uint8 aoeMaxTime;
	Uint8 aoeSide;

	GFC_Color* symbolMons;
	Uint8 symbolMonsColor;
	Uint8 symbolOrder;		//Abstract int. Increments when the correct color is killed. If it equals the num of minions spawned, determins the phase ending! That num is hardcoded atm.
	Entity* symbolMonster1;
	Entity* symbolMonster2;
	Entity* symbolMonster3;

	Uint8 boss3AttackTimer;
	Uint8 boss3AttackMaxTime;
	Uint8 bossAttack;
	Uint16 bossSnipeMax;
	Uint16 bossSnipeCount;
	Uint16 bossNukeMax;
	Uint16 bossNukeCount;
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
void symbolPatternAlert(Entity* self, GFC_Color color);
void boss3Attack(Entity* self);
void visualizeHitbox(Entity* self);
void randomPuzzle(Entity* self);



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

	self->hp = 10;
	self->maxHP = self->hp;

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
	monsterData->phase = MP_HEALTHY_ONCE;

	switch (role)
	{
		case ROLE_BOSS1:
			monsterData->state = MS_IDLE;
			monsterData->phaseCount = 0;
			self->layer = EL_BOSS;

			monsterData->puzzle1 = cupShoot;
			monsterData->puzzle2 = symbols;

			monsterData->symbol1 = NULL;
			monsterData->symbol2 = NULL;
			setBoss(self);
			break;

		case ROLE_BOSS2:
			self->layer = EL_BOSS;
			monsterData->state = MS_IDLE;
			monsterData->phaseCount = 0;
			self->timeToLive = 500;

			monsterData->aoeMaxTime = 10;
			monsterData->aoeTimer = 0;

			monsterData->puzzle1 = aoe;
			monsterData->puzzle2 = symbolPattern;

			monsterData->symbolMons = NULL;
			monsterData->symbolMonsColor = 0;
			monsterData->symbolOrder = 0;

			setBoss(self);
			break;

		case ROLE_BOSS3:
			self->layer = EL_BOSS;
			self->timeToLive = 500;

			monsterData->puzzle1 = randomPuzzle;
			monsterData->puzzle2 = randomPuzzle;

			monsterData->boss3AttackTimer = 0;
			monsterData->boss3AttackMaxTime = 50;

			monsterData->bossAttack = 0;
			monsterData->state = MS_IDLE;

			monsterData->bossSnipeCount = 0;
			monsterData->bossSnipeMax = 500;
			//slog("Monster Data BossSnipeMax: %i",monsterData->bossSnipeMax);
			
			monsterData->bossNukeCount = 0;
			monsterData->bossNukeMax = 50;

			monsterData->symbolMons = NULL;
			monsterData->symbolMonsColor = 0;
			monsterData->symbolOrder = 0;

			self->timeToLive = 500;

			monsterData->aoeMaxTime = 10;
			monsterData->aoeTimer = 0;

			monsterData->symbol1 = NULL;
			monsterData->symbol2 = NULL;

			setBoss(self);
			break;

		case ROLE_SYMBOL_ENEMY1:
			//self->layer = EL_MONSTER;
			self->colorReal = GFC_COLOR_DARKRED;
			self->layer = EL_MONSTER;
			self->hp = 1;
			monsterData->state = MS_TRASH;
			theBoss = getBoss();

			break;

		case ROLE_SYMBOL_ENEMY2:
			self->colorReal = GFC_COLOR_DARKYELLOW;
			self->layer = EL_MONSTER;
			self->hp = 1;
			monsterData->state = MS_TRASH;
			theBoss = getBoss();
			break;

		case ROLE_SYMBOL_ENEMY3:
			self->colorReal = GFC_COLOR_DARKBLUE;
			self->layer = EL_MONSTER;
			self->hp = 1;
			monsterData->state = MS_TRASH;
			theBoss = getBoss();
			break;

		default: //Trashmod
			monsterData->state = MS_TRASH;
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
			//slog("Player aligned thing touched me %s. HP is now %i",self->name,self->hp);
		}
	}
}

void monsterUpdate(Entity* self)
{
	if (!self)
		return;

	//slog("Updateing Monster!");
	MonsterData* data = (MonsterData*)self->data;

	if (!data)
	{
		slog("Monster Update: Monster has no data! Killing it!");
		self->_inUse = 0;
		return;
	}

	self->frame += 0.1;
	if (self->frame >= 8)
		self->frame = 0;

	switch(data->state)
	{
		case NULL:
			slog("Monster State is NULL!");

		case MS_DEAD:
			self->_inUse = 0;
			return;

		case MS_TEST:
			visualizeHitbox(self);
			return;

		case MS_ATTACK:

			//Temporary Boss 1 stock attack
			trashShoot(self,getPlayer());
			data->state = MS_IDLE;
			self->timerPrimary = 0;
			break;

		case MS_IDLE:
			break;

		case MS_PUZZLE_WAIT:
			self->isInvul = 1;
			//slog("Waiting for Puzzle!");

			break;
		case MS_PUZZLE1:
			data->puzzle1(self);
			data->state = MS_PUZZLE_WAIT;

			break;

		case MS_PUZZLE2:
			data->puzzle2(self);
			data->state = MS_PUZZLE_WAIT;

			break;

		default:
			slog("Boss Update State does not exist! Current state: %i",data->state);
	}

	visualizeHitbox(self);
}

void monsterThink(Entity* self)
{
	//GFC_Vector2D toPlayer = { 0 };
	//MonsterData *data;

	if (!self)
		return;
	
	MonsterData* data = (MonsterData*)self->data;

	if (!data)
	{
		slog("Monster somehow has no data! Killing it!");
		self->_inUse = 0;
		return;
	}

	if (data->state == MS_TEST)
	{
		return;
	}

	if (self->layer != EL_INVISIBLE)
	{
		//Boss 1 logic
		if (!data->player)
		{
			slog("I do not know about the player!");

		}
		else
		{
			switch (data->phase)
			{
				case MP_HEALTHY_ONCE:
					slog("Boss is currently Healthy Once!");
					//Let the boss talk, animate, etc... play out
					//Boss has none of that atm, so just make them healthy
					data->phase = MP_HEALTHY;


					data->phase = MP_NEAR_DEATH_ONCE;
					break;

				case MP_HEALTHY:
					//Check if HP is below threshold, if yes return phase 2 entrance
					//slog("Boss is currently Healthy!");
					//data->phase = MP_INJURED_ONCE;
					//data->state = MS_IDLE;

					if (!self->hp)
						slog("Boss has no HP?!");

					if (!self->maxHP)
						slog("Boss has no Max HP?!");

					if (self->hp <= self->maxHP * 0.8)
					{
						slog("Boss is no longer healthy!");
						data->phase = MP_INJURED_ONCE;
						data->state = MS_IDLE;
					}
					break;
				case MP_INJURED_ONCE:
					slog("Boss is currently Injured Once!");
					//To be used by the Boss once to trigger puzzle 1
					data->state = MS_PUZZLE1;
					data->phase = MP_INJURED;
					break;
				case MP_INJURED:
					//slog("Boss is currently Injured!");
					//Check HP threshold
					if (self->hp <= self->maxHP * healthStates[HS_INJURED])
					{
						data->phase = MP_NEAR_DEATH_ONCE;
						data->state = MS_IDLE;
					}
					break;
				case MP_NEAR_DEATH_ONCE:
					//Do puzzle
					slog("Boss is currently Near Death Once!");
					data->state = MS_PUZZLE2;
					data->phase = MP_NEAR_DEATH;

					break;
				case MP_NEAR_DEATH:
					//slog("Boss is currently Near Death!");
					//Do normal boss stuff - Death is checked elsewhere

					break;

				default:
					slog("No phase data from Boss!");

			
			}

			//slog("Current boss phase is: %i", data->phase);
			if (data->state == MS_IDLE)
			{
					
				if (self->timerPrimary >= self->primaryCooldown)
				{
					slog("Boss is changing state to Attack!");
					data->state = MS_ATTACK;
				}
				else
					self->timerPrimary++;
			}

		}

	}
	/*else if (self->layer != EL_INVISIBLE && self->role == ROLE_BOSS2)
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
				if (((MonsterData*)self->data)->state == MS_IDLE && ((MonsterData*)self->data)->phaseCount == 0)
				{
					//((MonsterData*)self->data)->phaseCount++;
					//slog("AOE!");
					aoe(self);
				}
				else if (((MonsterData*)self->data)->state == MS_IDLE && ((MonsterData*)self->data)->phaseCount == 1)
				{
					//((MonsterData*)self->data)->phaseCount++;
					symbolPattern(self);
				}
				else
				{
					
					
					if (((MonsterData*)self->data)->state == MS_AOE || ((MonsterData*)self->data)->state == MS_SYMBOLS_MOBS)
					{
						self->isInvul = 1;
					}

					if (((MonsterData*)self->data)->state == MS_SYMBOLS_MOBS)
					{
						//slog("Changing colors!");

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

					if (((MonsterData*)self->data)->state == MS_IDLE && ((MonsterData*)self->data)->phaseCount >= 2)
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
	else if (self->layer != EL_INVISIBLE && self->role == ROLE_BOSS3)
	{
		//Boss 3 logic... simple isn't it?

		//slog("Boss3 is attacking!");
		boss3Attack(self);
		
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
	
	//slog("Inside monster thinking. HP is %i", self->hp);*/
	
	if (self->hp <= 0)
	{
		if (self->role == ROLE_SYMBOL_ENEMY1 || self->role == ROLE_SYMBOL_ENEMY2 || self->role == ROLE_SYMBOL_ENEMY3)
		{
			if (self->hp <= 0)
			{
				symbolPatternAlert(theBoss, self->colorReal);
			}
		}
		data->state = MS_DEAD;
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
	Originally a test function, used for the Boss3 attacks /Trash mobs probably
	Fires a stock projectile at the player entitty
*/
void trashShoot(Entity* self, Entity* player)
{
	if (!self || !player)
		return;

	//slog("Trash mob trying to shoot!");



	GFC_Vector2D angle = gfc_vector2d(player->position.x - self->position.x,player->position.y - self->position.y);
	gfc_vector2d_normalize(&angle);

	//self->timeToLive
	Entity* projectile = projectileEntityNew(gfc_vector2d(self->position.x + (self->bounds.w/2),self->position.y + (self->bounds.h/2)), TEAM_ENEMY, -1, ROLE_PROJECTILE);
	projectile->colorReal = GFC_COLOR_DARKMAGENTA;

	moveProjectileMob(projectile,angle);

}

/*
	Used by Boss 1 to spawn the 3 cups then turn invisible
	Boss puzzle 1
*/
void cupShoot(Entity* self)
{

	if (!self)
		return;
	//slog("Boss 1 Puzzle 1");

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
	//((MonsterData*)self->data)->phaseCount++; //Temporary to have the boss loop through attacks


	slog("Cup has updated Boss's state! ");//phaseCount ++");
}

/*
	Boss 1 Puzzle 2
*/
void symbols(Entity* self)
{
	int distance = 100;
	int num;
	Entity* sym1;
	Entity* sym2;
	MonsterData* data;

	if (!self)
		return;

	data = (MonsterData*)self->data;

	if (!data)
	{
		return;
	}

	//slog("Boss 1 Puzzle 2!");
	

	if (data->symbol1)
	{
		//slog("Old symbol1, freeing!");
		data->symbol1->_inUse = 0;
	}

	if (data->symbol2)
	{
		//slog("Old symbol2, freeing!");
		data->symbol2->_inUse = 0;
	}


	//slog("Creating new symbols!");
	sym1 = projectileEntityNew(gfc_vector2d(self->position.x - distance, self->position.y), TEAM_ENEMY, -1, ROLE_SYMBOL1);
	sym2 = projectileEntityNew(gfc_vector2d(self->position.x + distance + self->bounds.w, self->position.y), TEAM_ENEMY, -1, ROLE_SYMBOL2);

	if (!sym1 || !sym2)
	{
		//slog("Could not create symbols for Boss 1!");

		if (sym1)
			sym1->_inUse = 0;
		if (sym2)
			sym2->_inUse = 0;
	}

	data->symbol1 = sym1;
	data->symbol2 = sym2;

	num = rand() % 2 + 1;

	data->symbolNum = num;

	switch(num)
	{
		case 1:
			self->colorReal = GFC_COLOR_DARKMAGENTA;
			break;
		default:
			self->colorReal = GFC_COLOR_DARKYELLOW;
	}

	if (!data->symbol1 || !data->symbol2)
	{
		//slog("Boss Symbol attack failed to spawn symbols!");

		if (data->symbol1)
			data->symbol1->_inUse = 0;

		if (data->symbol2)
			data->symbol2->_inUse = 0;
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

	//((MonsterData*)self->data)->phaseCount++; 
	((MonsterData*)self->data)->state = MS_IDLE;
	//slog("Correct Cupt Loop?");
	//Tempoary, see cup
}


/*
	Called every frame Boss 2 is running the AOE phase
	Changes the boss' color to indicate whether an attack is coming from the left or right
	Gives the player aoeTimer seconds to prepare, then nukes the field
	Boss 2's Puzzle 1
*/
void aoe(Entity* self)
{
	int num, c;
	GFC_Vector2D pos;
	Entity* thing;
	MonsterData* data;

	if (!self)
		return;

	data = (MonsterData*)self->data;

	if(!data)
		return;


	if (data->aoeTimer > data->aoeMaxTime)
	{
		//Boss is ready to attack!
		//End this attack
		
	}
	else if (data->aoeTimer > 0)
	{
		//Check if the boss has given the player time to think, IE timer > 0
		slog("Current AOE time is: %i", data->aoeTimer);
		data->aoeTimer++;
		return;
	}
	else
	{
		//Give the player time to react to the attack + change boss color to either Red or Green

		num = rand() % 2 + 1;
		data->aoeSide = num;

		switch (num)
		{
			case 1:
				self->colorReal = GFC_COLOR_RED;
				break;

			default :
				self->colorReal = GFC_COLOR_GREEN;
				break;
		}
		data->aoeTimer++;
		return;
	}

	self->colorReal = GFC_COLOR_TRANSPARENT;
	pos = gfc_vector2d(self->position.x,self->position.y);
	data->aoeTimer = 0;
	data->phaseCount++;
	data->state = MS_IDLE;

	//Teleport the boss to the center of the arena 1200 x 720
	//Then glow either green or red (left - right) 
	//Then enter a waiting period, once the waiting is over, do the actual AOE

	
	switch(data->aoeSide)
	{
		case 1:
			//Spawn a ton of projectile moving down to the left!
			//Just going to spawn a bunch of projectiles for 500 units to the left

			for (c = 0; c < 20; c++)
			{
				thing = projectileEntityNew(gfc_vector2d(self->position.x + c*50, self->position.y ), TEAM_ENEMY, self->timeToLive, ROLE_PROJECTILE);
				moveProjectile(thing, D_SOUTH);
			}
			break;

		default:
			//Spawn a ton of projectile moving down to the right!


			for (c = 0; c < 20; c++)
			{
				thing = projectileEntityNew(gfc_vector2d(self->position.x - c * 50, self->position.y), TEAM_ENEMY, self->timeToLive, ROLE_PROJECTILE);
				moveProjectile(thing, D_SOUTH);
			}

			break;
	}


}

/*
	Boss 2's symbol attack
	Flashes 3 colors, must kill the monsters in that order, fail and they explode, succeed and the boss is damagble
	Boss 2 Puzzle 2
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

	((MonsterData*)self->data)->symbolMonster1 = mon1;
	((MonsterData*)self->data)->symbolMonster2 = mon2;
	((MonsterData*)self->data)->symbolMonster3 = mon3;
	
	((MonsterData*)self->data)->state = MS_SYMBOLS_MOBS;

}

/*
	Helper Function called by the Symbol Monster Boss attack's symbol mobs to check if the mobs were shot in the right order
*/
void symbolPatternAlert(Entity* self, GFC_Color color)
{
	if (!self)
	{
		return;
	}
	Entity* bomb = NULL;
	int bombDuration = 200;

	if (color.a == ((MonsterData*)self->data)->symbolMons[((MonsterData*)self->data)->symbolOrder].a)
	{
		((MonsterData*)self->data)->symbolOrder++;
	}
	else
	{
		slog("User messed the order up! Kill all symbolMonsters by explosion and end the phase!");


		if (!((MonsterData*)self->data)->symbolMonster1)
		{
			;
		}
		else
		{
			bomb = bombEntityNew(((MonsterData*)self->data)->symbolMonster1->position,TEAM_ENEMY, bombDuration);

			if (!bomb)
			{
				slog("Failed to make bomb 1 in Boss 2 Symbol Monster Failed!");
			}

			((MonsterData*)self->data)->symbolMonster1->_inUse = 0;
			((MonsterData*)self->data)->symbolMonster1 = NULL;
		}
		//slog("Monster 1 dealt with!");

		if (!((MonsterData*)self->data)->symbolMonster2)
		{
			;
		}
		else
		{
			bomb = bombEntityNew(((MonsterData*)self->data)->symbolMonster2->position, TEAM_ENEMY, bombDuration);

			if (!bomb)
			{
				slog("Failed to make bomb 2 in Boss 2 Symbol Monster Failed!");
			}

			((MonsterData*)self->data)->symbolMonster2->_inUse = 0;
			((MonsterData*)self->data)->symbolMonster2 = NULL;
		}
		//slog("Monster 2 dealt with!");

		if (!((MonsterData*)self->data)->symbolMonster3)
		{
			;
		}
		else
		{
			bomb = bombEntityNew(((MonsterData*)self->data)->symbolMonster3->position, TEAM_ENEMY, bombDuration);

			if (!bomb)
			{
				slog("Failed to make bomb 3 in Boss 2 Symbol Monster Failed!");
			}

			((MonsterData*)self->data)->symbolMonster3->_inUse = 0;
			((MonsterData*)self->data)->symbolMonster3 = NULL;
		}
		//slog("Monster 3 dealt with!");

		((MonsterData*)self->data)->state = MS_IDLE;
		((MonsterData*)self->data)->phaseCount++;
		self->colorReal = GFC_COLOR_TRANSPARENT;

		return;
	}

	if (((MonsterData*)self->data)->symbolOrder >= 3)//Hard coded 3 at the minute
	{
		//Attack is over!
		((MonsterData*)self->data)->state = MS_IDLE;
		((MonsterData*)self->data)->phaseCount++;
		self->colorReal = GFC_COLOR_TRANSPARENT;
	}
}

/*
	Attack handler for Boss 3.
	Rolls a random weighted number then does an attack
*/
void boss3Attack(Entity* self)
{
	if (!self)
		return;

	int num;
	int c;
	Entity* N;

	//slog("Boss 3 attack timer is: %i\nBoss3 attack timer max is %i", ((MonsterData*)self->data)->boss3AttackTimer, ((MonsterData*)self->data)->boss3AttackMaxTime);

	if (((MonsterData*)self->data)->boss3AttackTimer > ((MonsterData*)self->data)->boss3AttackMaxTime)
	{
		//Do 1 of the attacks!
		//((MonsterData*)self->data)->bossAttack = B3_NUKE;

		if (((MonsterData*)self->data)->bossAttack == B3_NOTHING)
		{
			//The free attack, IE The boss does nothing!
			slog("Lucky! Boss3 is doing nothing!");
			

		}
		else if (((MonsterData*)self->data)->bossAttack == B3_SNIPE)
		{
			//You get several fast balls!
			trashShoot(self, getPlayer());
			//((MonsterData*)self->data)->boss3AttackTimer = 0;
			((MonsterData*)self->data)->bossSnipeCount++;
			//slog("BossSnipeCount: %i",((MonsterData*)self->data)->bossSnipeCount);
			//slog("BossSnipeMax: %i", ((MonsterData*)self->data)->bossSnipeMax);

			if (((MonsterData*)self->data)->bossSnipeCount >= ((MonsterData*)self->data)->bossSnipeMax)
			{
				((MonsterData*)self->data)->state == MS_IDLE;
				((MonsterData*)self->data)->boss3AttackTimer = 0;
				((MonsterData*)self->data)->bossSnipeCount = 0;
				return;
			}
			return;
			
		}
		else
		{
			//slog("Boss Nuke!");
			//The boss is nuking the field a la Blue Baby in TBOI
			N=bombEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
			
			((MonsterData*)self->data)->bossNukeCount++;
			explode(N);

			if (((MonsterData*)self->data)->bossNukeCount >= ((MonsterData*)self->data)->bossNukeMax)
			{
				((MonsterData*)self->data)->state == MS_IDLE;
				((MonsterData*)self->data)->boss3AttackTimer = 0;
				((MonsterData*)self->data)->bossNukeCount = 0;
				N->_inUse = 0;
				return;
			}
			N->_inUse = 0;

			return;
			

			
			
		}

		((MonsterData*)self->data)->state == MS_IDLE;
		((MonsterData*)self->data)->boss3AttackTimer = 0;
		return;
	}
	else if (((MonsterData*)self->data)->boss3AttackTimer > 0)
	{
		//Check if the boss has given the player time to think, IE timer > 0
		//slog("Current Boss3 time is: %i", ((MonsterData*)self->data)->boss3AttackTimer);
		((MonsterData*)self->data)->boss3AttackTimer++;
		return;
	}
	else
	{
		//Start the attack
		num = rand() % 10;
		//slog("Starting Boss 3 Attack!");

		if (num <= 2)
		{
			//Do nothing
			((MonsterData*)self->data)->bossAttack = B3_NOTHING;
		}
		else if (num > 2 && num <= 6)
		{
			//Snipe the player
			((MonsterData*)self->data)->bossAttack = B3_SNIPE;

		}
		else
		{
			//Nuke the area
			((MonsterData*)self->data)->bossAttack = B3_NUKE;
		}
		//slog("Boss3 attack has been primed!");
		((MonsterData*)self->data)->boss3AttackTimer++;
		((MonsterData*)self->data)->state = MS_MATH;
	}
	
}
/*
	Used by Boss 3 to random choose a puzzle to do
*/
void randomPuzzle(Entity self)
{
	int num;
	num = rand() % 4;

	switch (num)
	{
		case 0:

			break;

		case 1:

			break;

		case 2:

			break;

		default:

			break;
	}
}

/*
	Paints a red box around the hitbox
*/
void visualizeHitbox(Entity* self)
{
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

void setMonsterState(Entity* self, int newState)
{
	((MonsterData*)self->data)->state = newState;
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


