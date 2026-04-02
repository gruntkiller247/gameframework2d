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
	MS_TRASH = 1,//Default, should never change if trashmob
	MS_IDLE,	//Default behavior for Boss: if equals this make them randomly select an attack from the stuff below!
	MS_CUP,		//Spawn 3 cups and turn invisible/invul until real cup is killed!
	MS_SYMBOLS, //Spawn 3 symbols(colored projectiles) Player must stand on the one the boss is colored! ->Boss is invul during this!
	MS_AOE,
	MS_SYMBOLS_MOBS,
	MS_MATH,
	MS_ATTACK,
	MS_DEAD,
	MS_PUZZLE_WAIT,
	MS_PUZZLE_WAIT2,
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

	int canMove;
	int lastDirection;
	int currentDirection;
	Uint8 moveTimer;
	Uint8 moveMaxTime;

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
void moveRandom(Entity* self, int direction);
void move(Entity* self,int direction);
void moveStop(Entity* self);

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
	self->topSpeed = gfc_vector2d(10, 10);
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
	monsterData->canMove = 1;

	switch (role)
	{
		case ROLE_BOSS1:
			monsterData->state = MS_IDLE;
			monsterData->phaseCount = 0;
			self->layer = EL_BOSS;

			monsterData->puzzle1 = cupShoot;
			monsterData->puzzle2 = symbols;

			monsterData->aoeTimer = 0;

			monsterData->symbol1 = NULL;
			monsterData->symbol2 = NULL;
			setBoss(self);
			break;

		case ROLE_BOSS2:
			self->layer = EL_BOSS;
			monsterData->state = MS_IDLE;
			monsterData->phaseCount = 0;
			self->timeToLive = 500;

			monsterData->aoeMaxTime = 100;
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

			monsterData->puzzle1 = NULL;
			monsterData->puzzle2 = NULL;

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

			monsterData->aoeMaxTime = 100;
			monsterData->aoeTimer = 0;

			monsterData->symbol1 = NULL;
			monsterData->symbol2 = NULL;


			self->data = monsterData;
			randomPuzzle(self);

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
		
	monsterData->moveMaxTime = 200;
	monsterData->moveTimer = monsterData->moveMaxTime;

	monsterData->lastDirection = rand()% D_MAX;
	monsterData->currentDirection = rand() % D_MAX;

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

	int num;

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

		case MS_TRASH:
			//At some point they will do something!
			break;

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


			if (data->aoeTimer && data->aoeTimer > 0 && data->aoeTimer <= data->aoeMaxTime+1)
			{
				//Either Boss 2 or 3 doing Boss2 Puzzle 1
				data->state = MS_PUZZLE1;
			}			
			

			break;

		case MS_PUZZLE_WAIT2:
			self->isInvul = 1;
			slog("Puzzle Wait 2!");

			

			if (self->role != ROLE_BOSS1 && data->symbolMons)
			{
				//slog("Boss 2 Puzzle 2 Color swaping!");
				slog("SymbolMonsColor: %i",data->symbolMonsColor);
				
				//This is arbitray bullshit, but I could not think of another way to slow down the color swapping after
				//I made this entire system faster!
				if (data->symbolMonsColor <= 40)
				{
					self->colorReal = data->symbolMons[0];
				}
				else if (data->symbolMonsColor <= 80)
				{
					self->colorReal = data->symbolMons[1];
				}
				else if (data->symbolMonsColor <= 120)
				{
					self->colorReal = data->symbolMons[2];
				}
				else if (data->symbolMonsColor <= 160)
				{
					self->colorReal = data->symbolMons[3];
				}
				else if(data->symbolMonsColor > 200)
				{
					data->symbolMonsColor = 0;
				}
				else
				{
					
				}
				data->symbolMonsColor++;

			}
			else
				slog("Monster does not have knowledge of color array for Boss attack!");

			break;
		case MS_PUZZLE1:
			data->puzzle1(self);
			data->state = MS_PUZZLE_WAIT;

			break;

		case MS_PUZZLE2:
			data->puzzle2(self);
			data->state = MS_PUZZLE_WAIT2;

			break;

		default:
			slog("Boss Update State does not exist! Current state: %i",data->state);
	}

	if (data->canMove && data->moveTimer >= data->moveMaxTime)
	{
		data->moveTimer = 0;
		//slog("Boss is moving!");
		move(self, data->currentDirection);
		//moveStop(self);
	}
	else
		data->moveTimer++;
	
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

	if (self->layer != EL_INVISIBLE && (self->role == ROLE_BOSS1 || self->role == ROLE_BOSS2 || self->role == ROLE_BOSS3))
	{
		//Boss logic
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

					//data->phase = MP_INJURED_ONCE;
					//data->phase = MP_NEAR_DEATH_ONCE;
					break;

				case MP_HEALTHY:
					//Check if HP is below threshold, if yes return phase 2 entrance
					//slog("Boss is currently Healthy!");
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
	else
	{
		//Trash Mob/Random mobs spawned in Behavior
	}

	//Movement Think
	
	switch(data->lastDirection)
	{
		case D_NORTH:

			moveRandom(self,D_NORTH);

			break;

		case D_NORTHEAST:
			moveRandom(self, D_NORTHEAST);
			break;

		case D_EAST:
			moveRandom(self, D_EAST);
			break;

		case D_SOUTHEAST:
			moveRandom(self, D_SOUTHEAST);
			break;

		case D_SOUTH:
			moveRandom(self, D_SOUTH);
			break;

		case D_SOUTHWEST:
			moveRandom(self, D_SOUTHWEST);
			break;

		case D_WEST:
			moveRandom(self, D_WEST);
			break;

		case D_NORTHWEST:
			moveRandom(self, D_NORTHWEST);
			break;

		default:
			slog("Monster does not have Last Direction!");
	}

	if (self->velocity.y)
	{
		self->position.y += self->velocity.y;
	}

	if (self->velocity.x)
	{
		self->position.x += self->velocity.x;
	}

	if (self->velocity.y || self->velocity.x)
	{
		gfc_vector2d_normalize(&self->velocity);
	}
	

	
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
}

void monsterFree(Entity* self)
{

	if (!self)
		return;

	if(self->data)
		free(self->data);

	if (self->sprite)
		gf2d_sprite_free(self->sprite);


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

	if (self->layer == EL_INVISIBLE) 
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

	if (!self)
		return;

	((MonsterData*)self->data)->state=MS_IDLE;
	//((MonsterData*)self->data)->phase=((MonsterData*)self->data)->phase++;
	self->layer = EL_BOSS;


	slog("Cup has updated Boss's state! ");
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

	//slog("Sym1 and 2 created!");

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
		slog("Attacking!");
		
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

		//slog("Boss 2 Puzzle 1 Happening!");
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

	//TO DO - Update Boss Position!
	pos = gfc_vector2d(self->position.x,self->position.y);
	
	
	data->aoeTimer = 0;
	//data->state = MS_IDLE;

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

	data->state = MS_IDLE;


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
	GFC_Color shuffler[4];	//Array of 3 colors to be shuffled then given to the boss to cycle between the colors during this puzzle!
	Entity* mon1;
	Entity* mon2;
	Entity* mon3;
	MonsterData* data;

	data = ((MonsterData*)self->data);

	if (!data)
		return;

	if (!data->symbolMons)
	{
		;//slog("Monster has no data for SymbolMons: Good!");
	}
	else
	{
		free(((MonsterData*)self->data)->symbolMons);
		((MonsterData*)self->data)->symbolMons = NULL;
	}

	//slog("Boss 2 Puzzle 2!");

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
	shuffler[3] = self->colorReal;
	


	for (c = 0; c < sizeof(shuffler) / sizeof(GFC_Color); c++) 
	{
		
		d = rand() % sizeof(shuffler) / sizeof(GFC_Color);
		temp = shuffler[c];
		shuffler[c] = shuffler[d];
		shuffler[d] = temp;
	}


	if (!data->symbolMons)
	{
		data->symbolMons = malloc(sizeof(GFC_Color) * 4);

		if (!data->symbolMons)
		{
			slog("error allocating memory to symbolMons!");
			mon1->_inUse = 0;
			mon2->_inUse = 0;
			mon3->_inUse = 0;

			return;
		}
		memcpy(data->symbolMons, shuffler, sizeof(GFC_Color) * 4);
	}
	else
	{
		//data->symbolMons = malloc(sizeof(GFC_Color) * 4);
		memcpy(data->symbolMons, shuffler, sizeof(GFC_Color) * 4);
	}

	data->symbolMonster1 = mon1;
	data->symbolMonster2 = mon2;
	data->symbolMonster3 = mon3;
	
	if (self->role == ROLE_BOSS3)
	{
		data->state = MS_PUZZLE2;
	}

	

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
		self->colorReal = GFC_COLOR_TRANSPARENT;

		free(((MonsterData*)self->data)->symbolMons);
		return;
	}

	if (((MonsterData*)self->data)->symbolOrder >= 3)//Hard coded 3 at the minute
	{
		//Attack is over!
		((MonsterData*)self->data)->state = MS_IDLE;
		self->colorReal = GFC_COLOR_TRANSPARENT;
		free(((MonsterData*)self->data)->symbolMons);
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
void randomPuzzle(Entity* self)
{
	int num;
	num = rand() % 4;
	MonsterData* data;
	slog("Random!");

	if (!self)
	{
		slog("Can't randomize Boss 3 as it's NULL! Killing it!");
		self->_inUse = 0;
	}

	data = (MonsterData*)self->data;
	if (!data)
	{
		slog("Boss 3 has no data! Killing it!");
		self->_inUse = 0;
	}

	num = 3;

	switch (num)
	{
		case 0:

			data->puzzle1 = cupShoot;
			slog("Rolled CupShoot");
			break;

		case 1:
			data->puzzle1 = symbols;
			slog("Rolled Symbols!");
			break;

		case 2:
			data->puzzle1 = aoe;
			slog("Rolled AOE");
			break;

		default:
			data->puzzle1 = symbolPattern;
			slog("Rolled Symbol Pattern");
			break;
	}

	num = rand() % 4;
	num = 3;

	switch (num)
	{
	case 0:

		data->puzzle2 = cupShoot;
		slog("Rolled CupShoot");
		break;

	case 1:
		data->puzzle2 = symbols;
		slog("Rolled Symbols!");
		break;

	case 2:
		data->puzzle2 = aoe;
		slog("Rolled AOE");
		break;

	default:
		data->puzzle2 = symbolPattern;
		slog("Rolled Symbol Pattern");
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

/*
	Code that actually moves the Monster
*/
void move(Entity* self, int direction)
{
	if (!self)
		return;

	//slog("Monster is trying to move! Direction is %i",direction);

	switch (direction)
	{
	case(D_NORTH):
		self->velocity.y -= self->topSpeed.y;
		//slog("Moving North!");
		break;

	case(D_NORTHEAST):
		self->velocity.y -= self->topSpeed.y;
		self->velocity.x += self->topSpeed.x;
		//slog("Moving North East!");
		break;

	case(D_EAST):
		self->velocity.x += self->topSpeed.x;
		//slog("Moving East!");
		break;

	case(D_SOUTHEAST):
		self->velocity.y += self->topSpeed.y;
		self->velocity.x += self->topSpeed.x;
		//slog("Moving South East!");
		break;

	case(D_SOUTH):
		self->velocity.y += self->topSpeed.y;
		//slog("Moving South");
		break;

	case(D_SOUTHWEST):
		self->velocity.y += self->topSpeed.y;
		self->velocity.x -= self->topSpeed.x;
		//slog("Moving South West!");
		break;

	case(D_WEST):
		self->velocity.x -= self->topSpeed.x;
		//slog("Moving West!");
		break;

	case(D_NORTHWEST):
		self->velocity.y -= self->topSpeed.y;
		self->velocity.x -= self->topSpeed.x;
		//slog("Moving North West!");
		break;

	default:

		slog("No real direction given for monster movement!");
		return;


	}
	return;
}


/*
	Sets the monster's current direction to a random direction with weight given to their current direction area
	Note: DOES NOT RANDOMLY MOVE THE MONSTER: It is weighted
*/
void moveRandom(Entity* self, int direction)
{
	if (!self)
		return;

	int num, temp, directionNew;
	num = (rand() % 120) + 1;
	

	if (num <= 10) 
	{
		//1-10 
		//10% chance to do a 180
		directionNew = direction + 4;



	}
	else if (num <= 30)
	{
		//11-40
		//30% chance to move back right or back left
		temp = rand() % 2;
		if (temp)
		{
			//Back Right
			directionNew = direction + 3;

		}
		else
		{
			//Back Left
			directionNew = direction - 3;
		}
		
	}
	else if (num <= 60)
	{
		//Left or Right
		temp = rand() % 2;
		if (temp)
		{
			//Right
			directionNew = direction + 2;

		}
		else
		{
			//Left
			directionNew = direction - 2;
		}

	}
	else if (num <= 100)
	{
		//Move front right or front left
		temp = rand() % 2;
		if (temp)
		{
			//Front Right
			directionNew = direction + 1;

		}
		else
		{
			//Front Left
			directionNew = direction + 1;
		}
	}
	else
	{
		//Move Straight
		directionNew = direction;
	}

	if (directionNew < 0)
		directionNew += D_MAX;
	else if (directionNew > D_MAX - 1)
		directionNew -= D_MAX;



	((MonsterData*)self->data)->currentDirection = directionNew;
	//slog("The movement direction rolled is: %i", directionNew);

	
	//The cursed algorhytm - O(23) vs chad O(1)
	/*int nums[23];
	int c, rnum;
	int leftNum, rightNum;

	//The direction we are already going, N (0), is weighted 4x
	for (c = 0; c < 4; c++)
	{
		//slog("C is: %i ", c);
		nums[c] = direction;
	}

	leftNum = direction - 1;
	rightNum = direction + 1;

	//If the input is N, then we go to -1, we mean NW (7)
	if (leftNum < 0)
		leftNum += D_MAX;

	if (rightNum > D_MAX-1)
		rightNum -= D_MAX;

	for (c; c < 8; c++)
	{
		//slog("C is: %i ", c);
		nums[c] = leftNum;
	}

	for (c; c < 12; c++)
	{
		//slog("C is: %i ", c);
		nums[c] = rightNum;
	}

	leftNum = direction - 2;
	rightNum = direction + 2;

	if (leftNum < 0)
		leftNum += D_MAX;

	if (rightNum > D_MAX-1)
		rightNum -= D_MAX;
	
	for (c; c < 15; c++)
	{
		//slog("C is: %i ", c);
		nums[c] = leftNum;
	}

	for (c; c < 18; c++)
	{
		//log("C is: %i ", c);
		nums[c] = rightNum;
	}

	leftNum = direction - 3;
	rightNum = direction + 3;

	if (leftNum < 0)
		leftNum += D_MAX;

	if (rightNum > D_MAX-1)
		rightNum -= D_MAX;

	for (c; c < 20; c++)
	{
		//slog("C is: %i ", c);
		nums[c] = leftNum;
	}

	for (c; c < 22; c++)
	{
		//slog("C is: %i ", c);
		nums[c] = rightNum;
	}

	//This is now the center num aka behind
	rightNum = direction + 4;

	if (leftNum < 0)
		leftNum += D_MAX;

	if (rightNum > D_MAX-1)
		rightNum -= D_MAX;

	nums[c] = rightNum;		//This should be 23


	for (c = 0; c < sizeof(nums) / sizeof(nums[0]); c++)
	{
		//slog("Value of array %i",nums[c]);
		//slog("Value of C: %i\n Value of array: %i",c,nums[c]);
	}

	rnum = nums[rand() % 23];

	//slog("The movement direction rolled is: %i", rnum);
	((MonsterData*)self->data)->currentDirection = rnum;*/
	

}


/*
	Stops the Monster from moving
*/
void moveStop(Entity* self) 
{
	self->velocity.x = 0;
	self->velocity.y = 0;
}

