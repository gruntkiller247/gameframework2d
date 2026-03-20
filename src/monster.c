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
	MS_PAIN,
	MS_DIE,
	MS_MAX
}MonsterStates;

typedef enum
{
	MP_TRASH,
	MP_IDLE,
	MP_ATTACK,
	MP_PUZZLE,
	MP_MAX
}MonsterPhase;

typedef struct MD
{
	Entity* player;
	MonsterStates state;
	MonsterPhase phase;
}MonsterData;

void monsterThink(Entity* self);
void monsterFree(Entity* self);
void monsterUpdate(Entity* self);
void monsterTouch(Entity* self,Entity* toucher);
void trashShoot(Entity* self, Entity* player);

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

	//self->data = gfc_allocate_array(sizeOf(struct MonsterData), 1);
	//if (data)


	strcpy(self->name, "MONSTER");

	strcpy(self->name, "MONSTER");

	MonsterData* monsterData = malloc(sizeof(MonsterData));
	MonsterPhase* monsterPhase = malloc(sizeof(MonsterPhase));

	if (!monsterData)
		return NULL;

	if (!monsterPhase)
		return NULL;

	monsterData->player = getPlayer();
	monsterData->state = MS_IDLE;

	if (role == ROLE_BOSS1 || role == ROLE_BOSS2 || role == ROLE_BOSS3)
		monsterData->phase = MP_IDLE;
	else
	{
		monsterData->phase = MP_TRASH;
		self->timeToLive = 800;
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

		if (toucher->team = TEAM_PLAYER && self->isInvul == 0)
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

	if (self->role == ROLE_TRASHMOB)
	{
		/*
		int timerPrimary;				//Timer that counts up to cooldown
		int primaryCooldown;			//Time until primary attack can be fired
		nt basicPlayerProjectileLife;  //Projectile timer to live cap for the Player
		*/

		if (!((MonsterData*)self->data)->player)
		{
			slog("I do not know about the player! %s");

		}
		else
		{
			//slog("I know about the player!");
			if (self->timerPrimary >= self->primaryCooldown)
			{
				trashShoot(self, ((MonsterData*)self->data)->player);
				self->timerPrimary = 0;
			}
			else
				self->timerPrimary++;
		}

	}
	else
	{
		if (!((MonsterData*)self->data)->player)
		{
			//slog("I do not know about the player and I am a boss!");
		}
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

	slog("Free Monster!");

	
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

void trashShoot(Entity* self, Entity* player)
{
	slog("Trash mob trying to shoot!");



	GFC_Vector2D angle = gfc_vector2d(player->position.x - self->position.x,player->position.y - self->position.y);
	gfc_vector2d_normalize(&angle);

	Entity* projectile = projectileEntityNew(gfc_vector2d(self->position.x + (self->bounds.w/2),self->position.y + (self->bounds.h/2)), TEAM_ENEMY, self->timeToLive);


	moveProjectileMob(projectile,angle);

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


