#include <SDL.h>
#include <SDL_mixer.h>
#include "simple_logger.h"
#include "entity.h"
#include "player.h"
#include "gfc_input.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "projectiles.h"
#include "bomb.h"



const char* monsterFile = "JSONs/monster.json";
const char* defaultShootingNoise = "audio/soundreality-laser-gun-280344.mp3";
const char* defaultDeathNoise = "audio/freesound_community-videogame-death-sound-43894.mp3";

typedef enum jsonArrayNum_S
{
	AN_TRASH = 0,
	AN_DODGE = 1,
	AN_BOSS1,
	AN_BOSS2,
	AN_BOSS3,
	AN_SYM1,
	AN_SYM2,
	AN_SYM3,
	AN_RUSH,
	AN_EXPLODE,
	AN_MOTHER,
	AN_CIRCLE,
	//All Monsters from Spring 2026 Class above

	AN_DEFAULT

}jsonArrayNum;


void loadMonster(Entity* self);
void shootLine(Entity* self);

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
	MS_ERROR = -1,
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
	MS_RUSH,
	MS_EXPLODE,
	MS_MOTHER,
	MS_CIRCLE,
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
	//Uint8 phaseCount;	//Does not keep track of the current phase, that is kept track by Monster Phases and checked in Think
	void (*puzzle1)(Entity* self);
	void (*puzzle2)(Entity* self);

	Uint32 bossID;		//ID of the 'Boss' of certain enemies

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

	Uint8 dodgeChance;
	Uint8 rush;
	int motherSpawn;
}MonsterData;


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

	MonsterData* monsterData = malloc(sizeof(MonsterData));
	//MonsterPhase* monsterPhase = malloc(sizeof(MonsterPhase));


	if (!monsterData)
		return NULL;

	self->role = role;
	self->position = position;

	monsterData->lastDirection = rand() % D_MAX;
	monsterData->currentDirection = rand() % D_MAX;

	self->data = monsterData;

	loadMonster(self);

	if (self->_inUse == 0)
		return NULL;

	
	


	monsterData->moveTimer = monsterData->moveMaxTime;
	self->frame = 0;
	self->think = monsterThink;
	self->update = monsterUpdate;
	self->free = monsterFree;
	self->touch = monsterTouch;
	self->velocity = gfc_vector2d(0, 0);
	self->rotation = 0;
	self->isInvul = 0;
	self->bounds = gfc_rect(30, 30, 72, 72);
	self->maxHP = self->hp;

	self->team = TEAM_ENEMY;
	monsterData->canMove = 1;
	monsterData->player = getPlayer();

	switch (role)
	{
	case ROLE_BOSS1:
		monsterData->phase = MP_HEALTHY_ONCE;
		break;
	case ROLE_BOSS2:
		monsterData->phase = MP_HEALTHY_ONCE;
		break;
	case ROLE_BOSS3:
		monsterData->phase = MP_HEALTHY_ONCE;
		break;
	default:
		;
	}
	

	return self;
}


void setMonsterBossID(Entity* self,int inID)
{
	if (!self)
		return;

	((MonsterData*)self->data)->bossID = inID;

}

void monsterTouch(Entity* self, Entity* toucher)
{
	int num;
	Entity* temp;

	if (!self || !toucher)
		return;

	if (!self->data)
		return;

	MonsterData* data = (MonsterData*)self->data;

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
		switch (self->role)
		{
			case ROLE_DODGE:

				num = rand() % (10 + 1);

				if (num < data->dodgeChance)
				{
					slog("Dodged!");
					if (self->hp < 0)
						self->isInvul = 1;
				}
				else
					self->hp -= toucher->damage;
				break;
			case ROLE_EXPLODE:

				if (toucher->team == TEAM_PLAYER && self->isInvul == 0)
				{
					temp = bombEntityNew(self->position,self->team,0);
					explode(temp);
					
					self->hp -= toucher->damage;

					if (self->hp < 0)
						self->isInvul = 1;
					slog("Player aligned thing touched me %s. HP is now %i",self->name,self->hp);
				}

				break;
			default:
				if (toucher->team == TEAM_PLAYER && self->isInvul == 0)
				{
					self->hp -= toucher->damage;
					//slog("Toucher's damage: %i", toucher->damage);

					if(self->hp < 0)
						self->isInvul = 1;
					
					slog("Player aligned thing touched me %s. HP is now %i",self->name,self->hp);
				}
		}

		if (toucher->team == TEAM_PLAYER && toucher != getPlayer())//(toucher->role != ROLE_PLAYER_BAKER || toucher->role != ROLE_PLAYER_GAMBLER || toucher->role != ROLE_PLAYER_GUNNER))
		{
			toucher->_inUse = 0;
		}



	}
}

void monsterUpdate(Entity* self)
{
	int num;

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
	//addTo(self);

	

	self->frame += 0.1;
	if (self->frame >= 8)
		self->frame = 0;

	switch(data->state)
	{
		case NULL:
			slog("Monster State is NULL!");

		case MS_DEAD:
			
			if (returnKilled() % 10 == 0)
			{
				addPlayerPoints((self->points + killedPoints + rngPoints()) * getScoreMult());
			}
			else
				addPlayerPoints((self->points + rngPoints())* getScoreMult());
			slog("Player Points: %i", getPlayerPoints());

			Mix_PlayChannel(3, self->deathChunk, 0);
			reportDeath();

			self->_inUse = 0;
			return;

		case MS_ERROR:
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
					self->color = data->symbolMons[0];
				}
				else if (data->symbolMonsColor <= 80)
				{
					self->color = data->symbolMons[1];
				}
				else if (data->symbolMonsColor <= 120)
				{
					self->color = data->symbolMons[2];
				}
				else if (data->symbolMonsColor <= 160)
				{
					self->color = data->symbolMons[3];
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

		case MS_RUSH:
			break;

		case MS_EXPLODE:
			break;

		case MS_MOTHER:
			//ROLE_PU_MIN+1 + rand() % (ROLE_PU_MAX-1-ROLE_PU_MIN);
			data->motherSpawn = ROLE_TRASHMOB;//ROLE_MOTHER_MIN + rand() % (ROLE_MOTHER_CAP-1-ROLE_MOTHER_MIN);
			//slog("Mother's Role to spawn is: %i");
			break;

		case MS_CIRCLE:
			//Spawn projectile circle
			//circleRing(self);

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

	if (data->state == MS_ERROR)
	{
		return;
	}

	if (self->hp <= 0)
	{
		if (self->role == ROLE_SYMBOL_ENEMY1 || self->role == ROLE_SYMBOL_ENEMY2 || self->role == ROLE_SYMBOL_ENEMY3)
		{

			if (!getBoss())
			{
				slog("Symbol failed to find boss pointer!");
				//data->state = MS_DEAD;
			}
			symbolPatternAlert(getBoss(), self->color);
			
		}
		data->state = MS_DEAD;
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
		//slog("Timer Primary %i\PrimaryCooldown: %i",self->timerPrimary,self->primaryCooldown);
		if (self->timerPrimary >= self->primaryCooldown)
		{
			if (self->role == MS_MOTHER && data->motherSpawn && ((rand() % 10 + 1) == 1))
			{
				//slog("TRYING TO SPAWN OFFSPRING FROM MOTHER!\n Mother spawn is role: %i",data->motherSpawn);
				monsterEntityNew(self->position, data->motherSpawn);
			}
			else if (self->role == ROLE_CIRCLE)
			{
				shootLine(self);
			}
			self->timerPrimary = 0;
		}
		else
			self->timerPrimary++;
		//Trash Mob/Random mobs spawned in Behavior

	}

	//Movement Think
	
	switch (self->role)
	{
		case ROLE_RUSH:
		

		case ROLE_EXPLODE:

			moveTowardsSpot(self, getPlayer());
			break;
	default: 
		switch (data->lastDirection)
		{
		case D_NORTH:

			moveRandom(self, D_NORTH);

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

	Mix_PlayChannel(2, self->fireChunk, 0);

	//self->timeToLive
	Entity* projectile = projectileEntityNew(gfc_vector2d(self->position.x + (self->bounds.w/2),self->position.y + (self->bounds.h/2)), TEAM_ENEMY, -1, ROLE_PROJECTILE);
	projectile->color = GFC_COLOR_DARKMAGENTA;

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

	setMonsterBossID(sym1,self->id);
	setMonsterBossID(sym2, self->id);

	data->symbol1 = sym1;
	data->symbol2 = sym2;

	num = rand() % 2 + 1;

	data->symbolNum = num;

	switch(num)
	{
		case 1:
			self->color = GFC_COLOR_DARKMAGENTA;
			break;
		default:
			self->color = GFC_COLOR_DARKYELLOW;
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

	self->color = GFC_COLOR_TRANSPARENT;

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
				self->color = GFC_COLOR_RED;
				break;

			default :
				self->color = GFC_COLOR_GREEN;
				break;
		}
		data->aoeTimer++;
		return;
	}

	self->color = GFC_COLOR_TRANSPARENT;

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

	shuffler[0] = mon1->color;
	shuffler[1] = mon2->color;
	shuffler[2] = mon3->color;
	shuffler[3] = self->color;
	


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
		self->color = GFC_COLOR_TRANSPARENT;

		free(((MonsterData*)self->data)->symbolMons);
		return;
	}

	if (((MonsterData*)self->data)->symbolOrder >= 3)//Hard coded 3 at the minute
	{
		//Attack is over!
		((MonsterData*)self->data)->state = MS_IDLE;
		self->color = GFC_COLOR_TRANSPARENT;
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

	MonsterData* data;
	//slog("Random!");

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

	if (!data->puzzle1 && !data->puzzle2)
		;
	else
		return;

	num = rand() % 4;
	//num = 3;
	

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
	//num = 3;

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

/*
	Recieves a monster Entity and a String of a Monster State from a JSON.
	Converts the stirng into a real Monster State then applies it to the Entity!
	Sets the Monster state to MS_ERROR if it does not find a state!
*/
void getMonsterState(Entity* self, const char* state)
{
	if (!state || !self)
		return;

	slog(state);

	if (strcmp(state, "MS_TRASH") == 0)
	{
		((MonsterData*)self->data)->state = MS_TRASH;
	}
	else if (strcmp(state, "MS_RUSH") == 0)
	{
		((MonsterData*)self->data)->state = MS_RUSH;
	}
	else if (strcmp(state, "MS_EXPLODE") == 0)
	{
		((MonsterData*)self->data)->state = MS_EXPLODE;
	}
	else if (strcmp(state, "MS_MOTHER") == 0)
	{
		((MonsterData*)self->data)->state = MS_MOTHER;
	}
	else if (strcmp(state, "MS_CIRCLE") == 0)
	{
		((MonsterData*)self->data)->state = MS_CIRCLE;
	}
	else if (strcmp(state, "MS_IDLE") == 0)
	{
		((MonsterData*)self->data)->state = MS_IDLE;
	}
	else if (strcmp(state, "MS_CUP") == 0)
	{
		((MonsterData*)self->data)->state = MS_CUP;
	}
	else if (strcmp(state, "MS_SYMBOLS") == 0)
	{
		((MonsterData*)self->data)->state = MS_SYMBOLS;
	}
	else if (strcmp(state, "MS_AOE") == 0)
	{
		((MonsterData*)self->data)->state = MS_AOE;
	}
	else if (strcmp(state, "MS_SYMBOLS_MOBS") == 0)
	{
		((MonsterData*)self->data)->state = MS_SYMBOLS_MOBS;
	}
	else if (strcmp(state, "MS_MATH") == 0)
	{
		((MonsterData*)self->data)->state = MS_MATH;
	}
	else if (strcmp(state, "MS_ATTACK") == 0)
	{
		((MonsterData*)self->data)->state = MS_ATTACK;
	}
	else if (strcmp(state, "MS_DEAD") == 0)
	{
		((MonsterData*)self->data)->state = MS_DEAD;
	}
	else if (strcmp(state, "MS_PUZZLE_WAIT") == 0)
	{
		((MonsterData*)self->data)->state = MS_PUZZLE_WAIT;
	}
	else if (strcmp(state, "MS_PUZZLE_WAIT2") == 0)
	{
		((MonsterData*)self->data)->state = MS_PUZZLE_WAIT2;
	}
	else if (strcmp(state, "MS_PUZZLE1") == 0)
	{
		((MonsterData*)self->data)->state = MS_PUZZLE1;
	}
	else if (strcmp(state, "MS_PUZZLE2") == 0)
	{
		((MonsterData*)self->data)->state = MS_PUZZLE2;
	}
	else
	{
		slog("Error reading Monster State from JSON!");
		((MonsterData*)self->data)->state = MS_ERROR;
	}
}

/*
	Recieves a monster Entity and a String of a Monster Layer from a JSON.
	Converts the string into a real Entity Layer then applies it to the entity!
	Sets the Entity Layer to EL_MONSTER if NULL!
*/
void getMonsterLayer(Entity* self, const char* layer)
{
	if (strcmp(layer, "EL_BOSS") == 0)
	{
		self->layer = EL_BOSS;
	}
	else
	{
		self->layer = EL_MONSTER;
	}
}

/*
	Recieves a monster Entity, a String of a Monster's Puzzle from a JSON, and a slot for the Puzzle number
	Converts the string into a real puzzle, then applies it to the Entity!
	Sets the Puzzle to NULL if it does not find a state!
*/
void getMonsterPuzzle(Entity* self, const char* puzzle, int puzzleNumber)
{
	if (strcmp(puzzle, "cupShoot") == 0)
	{
		if (puzzleNumber == 1)
			((MonsterData*)self->data)->puzzle1 = cupShoot;
		else
			((MonsterData*)self->data)->puzzle2 = cupShoot;

	}
	else if (strcmp(puzzle, "symbols") == 0)
	{
		if (puzzleNumber == 1)
			((MonsterData*)self->data)->puzzle1 = symbols;
		else
			((MonsterData*)self->data)->puzzle2 = symbols;

	}
	else if (strcmp(puzzle, "aoe") == 0)
	{
		if (puzzleNumber == 1)
			((MonsterData*)self->data)->puzzle1 = aoe;
		else
			((MonsterData*)self->data)->puzzle2 = aoe;

	}
	else if (strcmp(puzzle, "symbolPattern") == 0)
	{
		if (puzzleNumber == 1)
			((MonsterData*)self->data)->puzzle1 = symbolPattern;
		else
			((MonsterData*)self->data)->puzzle2 = symbolPattern;

	}
	else if (strcmp(puzzle, "randomPuzzle") == 0)
	{
		randomPuzzle(self);
	}
	else
	{
		if (puzzleNumber == 1)
			((MonsterData*)self->data)->puzzle1 = NULL;
		else
			((MonsterData*)self->data)->puzzle2 = NULL;
	}
		
}

void loadMonster(Entity* self)
{
	if (!self)
		return;

	MonsterData* data = (MonsterData*)self->data;

	SJson* json = NULL;
	SJson* mjson = NULL;
	SJson* roles = NULL;
	SJson* monster = NULL;
	
	const char* state = NULL;
	const char* layer = NULL;
	const char* spriteString = NULL;
	const char* puzzle1 = NULL;
	const char* puzzle2 = NULL;
	const char* name = NULL;

	const char* fireNoise = NULL;
	const char* deathNoise = NULL;

	int c = 0;
	int maxIndex = 0;

	int hp = 0;
	int damage = 0;
	int hitDelay = 0;
	int hitTimer = 0;
	int primaryCooldown = 0;
	int timerPrimary = 0;
	int topSpeedX = 0;
	int topSpeedY = 0;
	int moveMaxTime = 0;
	int timeToLive = 0;

	int aoeTimer = 0;
	int aoeMaxTimer = 0;

	int boss3AttackTimer = 0;
	int boss3AttackMaxTime = 0;
	int bossAttack = 0;
	int bossSnipeCount = 0;
	int bossSnipeMax = 0;
	int bossNukeCount = 0;
	int bossNukeMax = 0;

	int points = -1;
	int dodgeChance =-1;

	json = sj_load(monsterFile);

	if (!json)
	{
		slog("Failed to read Monster JSON!");
		goto fail;
	}

	mjson = sj_object_get_value(json, "monster");


	fireNoise = sj_object_get_string(mjson,"fireNoise");

	
	if (!fireNoise)
	{
		slog("Failed to load Monster's sound effect! Loading default!");
		self->fireSound = defaultShootingNoise;
		self->fireChunk = Mix_LoadWAV(self->fireSound);
	}
	else
	{
		self->fireSound = fireNoise;
		self->fireChunk = Mix_LoadWAV(self->fireSound);
	}

	deathNoise = sj_object_get_string(mjson,"deathNoise");

	if (!deathNoise)
	{
		slog("Failed to load Monster's death sound effect! Loading default!");
		self->deathSound = defaultDeathNoise;
		self->deathChunk = Mix_LoadWAV(self->deathSound);
	}
	else
	{ 
		self->deathSound = deathNoise;
		self->deathChunk = Mix_LoadWAV(self->deathSound);
	}
	

	if (!mjson)
	{
		slog("Failed to find monsters in JSON!");
		goto fail;
	}

	roles = sj_object_get_value(mjson, "roles");

	if (!roles)
	{
		slog("Failed to get array for Monster JSON!");
		goto fail;
	}

	if (gfc_list_get_count(roles) == 0)
	{
		slog("Roles list for monsters is 0?");
		goto fail;
	}

	

		switch (self->role)
		{
			case ROLE_TRASHMOB:
				monster = sj_array_get_nth(roles, AN_TRASH);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);
				

				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;
				

				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				break;

			case ROLE_EXPLODE:
				monster = sj_array_get_nth(roles, AN_EXPLODE);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				break;

			case ROLE_RUSH:

				monster = sj_array_get_nth(roles, AN_RUSH);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				break;

			case ROLE_DODGE:
				monster = sj_array_get_nth(roles, AN_DODGE);

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				if (sj_object_get_int(monster, "dodgeChance", &dodgeChance) == 0)
				{
					slog("Failed to get Monster's dodge chance!");
				}

				data->dodgeChance = dodgeChance;

				break;

			case ROLE_BOSS1:
				monster = sj_array_get_nth(roles, AN_BOSS1);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}


				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				//Puzzle 1 +2, AOE timer, AOE maxTime
				
				getMonsterPuzzle(self, sj_object_get_string(monster, "puzzle1"), 1);

				if (!data->puzzle1)
				{
					slog("Error reading Boss Puzzle 1!");
					goto fail;
				}

				getMonsterPuzzle(self, sj_object_get_string(monster, "puzzle2"), 2);

				if (!data->puzzle2)
				{
					slog("Error reading Boss Puzzle 2!");
					goto fail;
				}

				if (sj_object_get_int(monster, "aoeTimer", &aoeTimer) == 0)
				{
					slog("Error reading aoeTimer for Monster!");
					goto fail;
				}
				data->aoeTimer = aoeTimer;

				if (sj_object_get_int(monster, "aoeMaxTime", &aoeMaxTimer) == 0)
				{
					slog("Error reading aoeMaxTime for Monster!");
					goto fail;
				}
				data->aoeMaxTime = aoeMaxTimer;
				

				break;

			case ROLE_BOSS2:
				monster = sj_array_get_nth(roles, AN_BOSS2);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;


				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				//Puzzle 1 +2, AOE timer, AOE maxTime

				getMonsterPuzzle(self, sj_object_get_string(monster, "puzzle1"), 1);

				if (!data->puzzle1)
				{
					slog("Error reading Boss Puzzle 1!");
					goto fail;
				}

				getMonsterPuzzle(self, sj_object_get_string(monster, "puzzle2"), 2);

				if (!data->puzzle2)
				{
					slog("Error reading Boss Puzzle 2!");
					goto fail;
				}

				if (sj_object_get_int(monster, "aoeTimer", &aoeTimer) == 0)
				{
					slog("Error reading aoeTimer for Monster!");
					goto fail;
				}
				data->aoeTimer = aoeTimer;

				if (sj_object_get_int(monster, "aoeMaxTime", &aoeMaxTimer) == 0)
				{
					slog("Error reading aoeMaxTime for Monster!");
					goto fail;
				}
				data->aoeMaxTime = aoeMaxTimer;

				break;

			case ROLE_BOSS3:
				monster = sj_array_get_nth(roles, AN_BOSS3);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}


				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				//Puzzle 1 +2, AOE timer, AOE maxTime

				getMonsterPuzzle(self, sj_object_get_string(monster, "puzzle1"), 1);

				if (!data->puzzle1)
				{
					slog("Error reading Boss Puzzle 1!");
					goto fail;
				}

				getMonsterPuzzle(self, sj_object_get_string(monster, "puzzle2"), 2);

				if (!data->puzzle2)
				{
					slog("Error reading Boss Puzzle 2!");
					goto fail;
				}

				if (sj_object_get_int(monster, "aoeTimer", &aoeTimer) == 0)
				{
					slog("Error reading aoeTimer for Monster!");
					goto fail;
				}
				data->aoeTimer = aoeTimer;

				if (sj_object_get_int(monster, "aoeMaxTime", &aoeMaxTimer) == 0)
				{
					slog("Error reading aoeMaxTime for Monster!");
					goto fail;
				}
				data->aoeMaxTime = aoeMaxTimer;

				if (sj_object_get_int(monster, "boss3AttackTimer", &boss3AttackTimer) == 0)
				{
					slog("Error reading boss3AttackTimer for Monster!");
					goto fail;
				}
				
				data->boss3AttackTimer = boss3AttackTimer;


				if (sj_object_get_int(monster, "boss3AttackMaxTime", &boss3AttackMaxTime) == 0)
				{
					slog("Error reading boss3AttackTimer for Monster!");
					goto fail;
				}

				data->boss3AttackMaxTime = boss3AttackMaxTime;

				if (sj_object_get_int(monster, "bossAttack", &bossAttack) == 0)
				{
					slog("Error reading boss3AttackTimer for Monster!");
					goto fail;
				}

				data->bossAttack = bossAttack;
				
				if (sj_object_get_int(monster, "bossSnipeCount", &bossSnipeCount) == 0)
				{
					slog("Error reading bossSnipeCount for Monster!");
					goto fail;
				}

				data->bossSnipeCount = bossSnipeCount;

				if (sj_object_get_int(monster, "bossSnipeMax", &bossSnipeMax) == 0)
				{
					slog("Error reading bossSnipeMax for Monster!");
					goto fail;
				}

				data->bossSnipeMax = bossSnipeMax;

				if (sj_object_get_int(monster, "bossNukeCount", &bossSnipeMax) == 0)
				{
					slog("Error reading bossNukeCount for Monster!");
					goto fail;
				}

				data->bossNukeCount = bossNukeCount;

				if (sj_object_get_int(monster, "bossNukeMax", &bossNukeMax) == 0)
				{
					slog("Error reading bossNukeMax for Monster!");
					goto fail;
				}

				data->bossNukeMax = bossNukeMax;

				
				
				break;

			case ROLE_SYMBOL_ENEMY1:
				monster = sj_array_get_nth(roles, AN_SYM1);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;


				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				getColor(self, sj_object_get_string(monster, "color"));

				break;

			case ROLE_SYMBOL_ENEMY2:
				monster = sj_array_get_nth(roles, AN_SYM2);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;


				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				getColor(self, sj_object_get_string(monster, "color"));

				break;

			case ROLE_SYMBOL_ENEMY3:
				monster = sj_array_get_nth(roles, AN_SYM3);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;


				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				getColor(self, sj_object_get_string(monster, "color"));

				break;

			case ROLE_MOTHER:
				monster = sj_array_get_nth(roles, AN_MOTHER);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				
				break;

			case ROLE_CIRCLE:
				monster = sj_array_get_nth(roles, AN_CIRCLE);

				if (!monster)
				{
					slog("Failed to find monster in array!");
					goto fail;
				}

				if (sj_object_get_int(monster, "hp", &hp) == 0)
				{
					slog("Error getting Monster's HP value!");
					goto fail;
				}

				self->hp = hp;

				spriteString = sj_object_get_string(monster, "sprite");

				if (!spriteString)
				{
					slog("Error getting Monster's sprite!");
					goto fail;
				}

				self->sprite = gf2d_sprite_load_all(spriteString, 128, 128, 16, 0);


				if (sj_object_get_int(monster, "damage", &damage) == 0)
				{
					slog("Error getting Monster's damage value!");
					goto fail;
				}

				self->damage = damage;


				if (sj_object_get_int(monster, "hitDelay", &hitDelay) == 0)
				{
					slog("Error getting Monster's hitDelay value!");
					goto fail;
				}

				self->hitDelay = hitDelay;

				if (sj_object_get_int(monster, "hitTimer", &hitTimer) == 0)
				{
					slog("Error getting Monster's hitTimer value!");
					goto fail;
				}

				self->hitTimer = hitTimer;

				if (sj_object_get_int(monster, "primaryCooldown", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's primaryCooldown value!");
					goto fail;
				}

				self->primaryCooldown = primaryCooldown;

				if (sj_object_get_int(monster, "timerPrimary", &primaryCooldown) == 0)
				{
					slog("Error getting Monster's timerPrimary value!");
					goto fail;
				}

				self->timerPrimary = timerPrimary;


				if (sj_object_get_int(monster, "topSpeedX", &topSpeedX) == 0)
				{
					slog("Error getting Monster's topSpeedX value!");
					goto fail;
				}


				if (sj_object_get_int(monster, "topSpeedY", &topSpeedY) == 0)
				{
					slog("Error getting Monster's topSpeedY value!");
					goto fail;
				}


				self->topSpeed = gfc_vector2d(topSpeedX, topSpeedY);

				if (sj_object_get_int(monster, "moveMaxTime", &moveMaxTime) == 0)
				{
					slog("Error getting Monster's moveMaxTime value!");
					goto fail;
				}

				data->moveMaxTime = moveMaxTime;


				getMonsterState(self, sj_object_get_string(monster, "state"));


				if (sj_object_get_int(monster, "timeToLive", &timeToLive) == 0)
				{
					slog("Error getting Monster's timeToLive value!");
					goto fail;
				}

				self->timeToLive = timeToLive;

				getMonsterLayer(self, sj_object_get_string(monster, "layer"));

				break;

			default:
				slog("MONSTER LOADING: Cannot find that role!");
				slog("Role not found is name: %s Role: %i",self->name, self->role);
				


		
	}

	if (sj_object_get_int(monster, "points", &points) == 0)
	{
		//slog("Could not find points! Defaulting! Name: %s", self->name);
	}
	else
		self->points = points;
		




	sj_free(json);
	return;
	

	fail:

	self->_inUse = 0;
	slog("Error in Monster! Removing!");

	if (json)
		sj_free(json);

}


/*
	Shoots a line of projectiles
*/
void shootLine(Entity* self)
{
	Entity* proj = NULL;

	if (!self)
		return;

	proj = projectileEntityNew(self->position,TEAM_ENEMY,self->timeToLive,ROLE_PROJECTILE);

	if (!proj)
		return;

	moveTowardsSpot(proj, getPlayer());
}




