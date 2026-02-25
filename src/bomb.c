#include "entity.h"
#include "simple_logger.h"
#include "bomb.h"
#include "projectiles.h"
#include "gf2d_draw.h"
#include "gf2d_graphics.h"

void bombThink(Entity* self);

void bombTouch(Entity* self, Entity* toucher);

void bombUpdate(Entity* self);

void bombFree(Entity* self);

Entity* bombEntityNew(GFC_Vector2D position, Uint8 team, int* timeToLive)
{
	Entity* self;
	self = entityNew();

	if (!self)
	{
		slog("Failed to spawn a bomb! for team %s", team);
		return NULL;
	}

	/*self->color = malloc(sizeof(GFC_Color));
	*self->color = GFC_COLOR_DARKORANGE;*/

	self->team = team;

	self->sprite = gf2d_sprite_load_all("images/pointer.png", 128, 128, 16, 0);
	self->position = position;
	self->frame = 0;

	self->think = bombThink;
	self->free = bombFree;
	self->update = bombUpdate;
	self->touch = bombTouch;

	self->bounds = gfc_rect(0, 0, 32, 32);
	self->damage = 3;

	self->team = team;
	
	strcpy(self->name, "FRESH_BOMB!");

	if (!timeToLive)
	{
		//slog("No time to live in BOMB!");
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

	return self;
}


void bombThink(Entity* self)
{
	if (!self)
		return;

	if (self->timerDeath != -1)
	{
		self->timerDeath += 1;

		//slog("Timer Death: %i", self->timerDeath);


		if (self->timerDeath >= self->timeToLive)
		{

			explode(self);
			self->_inUse = false;
			//slog("I am the bomb and I am about to blow up!");
			return;
		}
	}

	if (self->velocity.y)
	{
		self->position.y += self->velocity.y;
		//slog("Boming moving on Y!");
	}

	if (self->velocity.x)
	{
		self->position.x += self->velocity.x;
		//slog("Bomb moving on X!");
	}

	if (self->velocity.y || self->velocity.x)
	{
		gfc_vector2d_normalize(&self->velocity);
	}

}

void bombTouch(Entity* self, Entity* toucher) 
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
		//slog("Bomb is touching something! %s",toucher->name);
	}
}

void bombUpdate(Entity* self)
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

void bombFree(Entity* self) 
{
	if (!self)
		return;

	//slog("Bomb is being killed!");

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


void explode(Entity* self)
{
	Entity* N = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* NE = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* E = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* SE = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* S = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* SW = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* W = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);
	Entity* NW = projectileEntityNew(gfc_vector2d(self->position.x + self->bounds.x, self->position.y + self->bounds.y), self->team, self->timeToLive);

	//N, NE, E, SE, S ,SW, W ,NW

	if (self->damage / 3 > 0)
	{
		N->damage = self->damage / 3;
		NE->damage = self->damage / 3;
		E->damage = self->damage / 3;
		SE->damage = self->damage / 3;

		S->damage = self->damage / 3;
		SW->damage = self->damage / 3;
		W->damage = self->damage / 3;
		NW->damage = self->damage / 3;
	}
	else
	{
		N->damage = 1;
		NE->damage = 1;
		E->damage = 1;
		SE->damage = 1;

		S->damage = 1;
		SW->damage = 1;
		W->damage = 1;
		NW->damage = 1;
	}

	move(N, D_NORTH);
	move(NE, D_NORTHEAST);
	move(E, D_EAST);
	move(SE, D_SOUTHEAST);

	move(S, D_SOUTH);
	move(SW, D_SOUTHWEST);
	move(W, D_WEST);
	move(NW, D_NORTHWEST);

	self->_inUse = 0;
}