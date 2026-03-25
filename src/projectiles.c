#include "simple_logger.h"
#include "projectiles.h"
#include "entity.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"
#include "monster.h"
#include "bomb.h"

typedef struct PD
{
	int* timerDeath;				//Timer to count up to timeToLive
	int* timeToLive;				//Time to Live for projectiles like things. Can be NULL;
}Projectile_Data; //Currently cut content

void symbolExplode(Entity* self);

Entity* boss = NULL;


Entity* projectileEntityNew(GFC_Vector2D position, Uint8 team, int* timeToLive,int role)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		slog("Failed to spawn a projectile!");
		return NULL;
	}

	self->role = role;

	/*Projectile_Data* data = malloc(sizeof(Projectile_Data));
	
	if (!data)
	{
		slog("Data couldn't be made for projectile!");
		return NULL;
	}*/
	switch (role)
	{
		case ROLE_FAKECUP:
			self->layer = EL_ENEMY_PROJETILES;
			break;

		case ROLE_CUP:
			self->layer = EL_ENEMY_PROJETILES;
			break;

		case ROLE_SYMBOL1:
			self->layer = EL_SYMBOLS;
			self->colorReal = GFC_COLOR_DARKMAGENTA;
			break;

		case ROLE_SYMBOL2:
			self->layer = EL_SYMBOLS;
			self->colorReal = GFC_COLOR_DARKYELLOW;
			break;

		default:
			self->layer = EL_PROJECTILES;
	}

	self->sprite = gf2d_sprite_load_all("images/pointer.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = projectileThink;
	self->free = projectileFree;
	self->update = projectileUpdate;
	self->touch = projectileTouch;

	self->velocity = gfc_vector2d(0, 0);
	self->topSpeed = gfc_vector2d(10, 10);
	self->rotation = 0;

	self->bounds = gfc_rect(0, 0, 32, 32);

	self->team = team;
	

	self->damage = 1; //hard code this for now, generic projectiles always deal 1! So does body contact!

	boss = getBoss();
	
	if (!timeToLive)
	{
		slog("No time to live in Projectiles!");
		self->timeToLive = 5;
		self->timerDeath = 0;
		/*data->timeToLive = 5;
		data->timerDeath = 0;*/
	}
	else if (timeToLive == -1)
	{
		//data->timerDeath = -1;
		self->timerDeath = -1;
	}
	else
	{
		self->timeToLive = timeToLive;
		self->timerDeath = 0;
		//data->timeToLive = timeToLive;
		//data-> timerDeath = 0;
	}
		

	//slog("Timerdeath: %i",self->timerDeath);

	
	strcpy(self->name, "PROJECTILE");

	return self;
}




void projectileThink(Entity* self)
{
	if (!self)
		return;

	//((Player_Data*)self)->basicPlayerProjectileLife

	//This is used for the Player Gunner Ult
	if (self->ultIs > 0 && self->ultIs <= 100)
	{
		self->scale = gfc_vector2d(self->ultIs, self->ultIs);
		self->bounds = gfc_rect(0, 0, 32 * self->ultIs, 32 * self->ultIs);
		self->ultIs += 1;
		//slog("My damage is %i", self->damage);
	}
	else if (self->ultIs > 100)
	{
		//Kill itself
		self->ultIs = 0;
		self->_inUse = 0;
		//slog("Gunner ult killing self!");
	}
	else
		;

	if (self->timerDeath != -1)
	{
		self->timerDeath += 1;

		//slog("Timer Death: %i", self->timerDeath);

		
		if (self->timerDeath >= self->timeToLive)
		{
			self->_inUse = false;
			//slog("I am going to heaven, my child!");
			return;
		}
	}
	
	//Update to make a legal check and move in update later
	if (self->velocity.y)
	{
		self->position.y += self->velocity.y;
	}

	if (self->velocity.x)
	{
		self->position.x+=self->velocity.x;
	}

	if (self->velocity.y || self->velocity.x)
	{
		gfc_vector2d_normalize(&self->velocity);
	}




}

void projectileTouch(Entity* self, Entity* toucher)
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
	
	slog("ProjectileTouch called! self role: %i, toucher role: %i", self->role, toucher->role);
	if (selfLeft < toucherRight && selfRight > toucherLeft && selfTop  < toucherBottom && selfBottom > toucherTop)
	{

		if (toucher->layer == EL_PLAYER)
		{
			//slog("I am a projectile touching the player!");

			if (self->role == ROLE_SYMBOL1 || self->role == ROLE_SYMBOL2)
			{
				slog("I am a symbol touching a player! in SYMBOLS");
			}
		}

		switch (self->role)
		{
			case ROLE_CUP:
				//slog("I am a cup and the toucher's role is: %i", toucher->role);
				//Real cup stuff
				if (toucher->team == TEAM_PLAYER)
				{
					//slog("Player aligned thing touched the CUP!");
					//slog("Real Cup interacted by Player!");

					if (boss)
					{
						cupStateUpdate(boss);
					}


					self->_inUse = 0;
				}
				break;

			case ROLE_FAKECUP:
				//Fake cup stuff
				if (toucher->team == TEAM_PLAYER)
				{

					//slog("Player aligned thing touched the CUP!");
					//slog("Fake Cup Interacted by Player!");
					cupExplode(self);
					
				}
				break;

			case ROLE_SYMBOL1:
				

				if (boss)
				{
					if (getSymbol(boss) == 2)
					{
						symbolExplode(self);
						slog("I am symbol 1");
					}
					else
					{
						//I am the correct Symbol!
						correctSymbol(boss);
						slog("I am symbol 1 COrrect");
						self->_inUse = 0;
					}
				}
				else
				{
					slog("No boss. I am a symbol 1 with no purpose!");
				}
				break;

			case ROLE_SYMBOL2:
				


				if (boss)
				{
					if (getSymbol(boss) == 1)
					{
						symbolExplode(self);
						slog("I am Symbol 2");
					}
					else
					{
						//I am the correct Symbol!
						correctSymbol(boss);
						slog("I am Symbol 2 Correct");
						self->_inUse = 0;
					}
				}
				else
				{
					slog("No boss. I am a symbol 2 with no purpose!");
				}
				break;

			default:
				break;
				//Standard Projectile, deal damage then die handled by the player!
		}

	}


	
}

void projectileUpdate(Entity* self)
{
	if (!self)
		return;

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

void projectileFree(Entity* self)
{
	if (!self)
		return;

	//slog("Projectile is being killed!");

	if (self->sprite)
	{
		gf2d_sprite_free(self->sprite);
	}
	
	if (self->data)
		free(self->data);

	/*if (self->color)
		free(self->color);*/

	free(self);
}

/*
	Used for the player, moves the projectile in one of the 4 cardinal directions
*/
void moveProjectile(Entity* self, int direction)
{
	if (!self)
		return;

	//slog("Inside Projectile Move! Entity Name: %s",self->name);

	switch (direction)
	{
		case(D_NORTH):
			self->velocity.y -= self->topSpeed.y;
			break;

		case(D_NORTHEAST):
			self->velocity.y -= self->topSpeed.y;
			self->velocity.x += self->topSpeed.x;
			break;

		case(D_EAST):
			self->velocity.x += self->topSpeed.x;
			break;

		case(D_SOUTHEAST):
			self->velocity.y += self->topSpeed.y;
			self->velocity.x += self->topSpeed.x;
			break;

		case(D_SOUTH):
			self->velocity.y += self->topSpeed.y;
			break;

		case(D_SOUTHWEST):
			self->velocity.y += self->topSpeed.y;
			self->velocity.x -= self->topSpeed.x;
			break;

		case(D_WEST):
			self->velocity.x -= self->topSpeed.x;
			break;

		case(D_NORTHWEST):
			self->velocity.y -= self->topSpeed.y;
			self->velocity.x -= self->topSpeed.x;
			break;

		default:

			slog("No real direction given for projectile movement!");
			return;

	
	}
	return;
}

/*
	Used by the AI, takes the self + a vector direction to move
*/
void moveProjectileMob(Entity* self, GFC_Vector2D direction)
{
	if (!self)
		return;

	self->velocity.x += direction.x;
	self->velocity.y += direction.y;
}

void gunnerUlt(Entity* self)
{
	if (!self)
		return;

	self->ultIs = 1;
}


void cupExplode(Entity* self)
{
	if (!self)
		return;

	slog("Fake cup is exploding!");
	Entity* bomb = bombEntityNew(self->position,TEAM_ENEMY,NULL);
	self->_inUse = 0;
}

void symbolExplode(Entity* self)
{
	if (!self)
		return;

	slog("Bad Symbol is exploding!");
	Entity* bomb = bombEntityNew(self->position, TEAM_ENEMY, NULL);
	self->_inUse = 0;
}
