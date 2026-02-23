#include "simple_logger.h"
#include "projectiles.h"
#include "entity.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"

typedef struct PD
{
	int* timerDeath;				//Timer to count up to timeToLive
	int* timeToLive;				//Time to Live for projectiles like things. Can be NULL;
}Projectile_Data; //Currently cut content


Entity* projectileEntityNew(GFC_Vector2D position, Uint8 team, int* timeToLive)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		slog("Failed to spawn a projectile!");
		return NULL;
	}

	/*Projectile_Data* data = malloc(sizeof(Projectile_Data));
	
	if (!data)
	{
		slog("Data couldn't be made for projectile!");
		return NULL;
	}*/

	self->sprite = gf2d_sprite_load_all("images/pointer.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = projectileThink;
	self->free = projectileFree;
	self->update = projectileUpdate;
	self->touch = projectileTouch;

	self->velocity = gfc_vector2d(0, 0);
	self->topSpeed = gfc_vector2d(100, 100);
	self->rotation = 0;

	self->bounds = gfc_rect(0, 0, 32, 32);

	self->team = team;

	
	
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

	if (selfLeft < toucherRight && selfRight > toucherLeft && selfTop  < toucherBottom && selfBottom > toucherTop)
	{
		//slog("Projectile is touching something!");
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

	slog("Projectile is being killed!");

	if (self->sprite)
	{
		gf2d_sprite_free(self->sprite);
	}
		

	free(self);
}